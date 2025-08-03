#include <string>
#include <vector>
#include <array>
#include <algorithm>
#include <cmath>

#include <Image.hpp>
#include <MathUtils.hpp>
#include <3D/GLTF/TextureInfo.hpp>
#include <3D/Vec.hpp>
#include <3D/meshGenerator.hpp>

#include <EvoAI/Utils/RandomUtils.hpp>

namespace fe{
    namespace priv{
        struct NoiseOptions{
            // Number of random noise points to add.
            unsigned int numPoints{32};
            // Minimum noise intensity (can be negative for darkening).
            int noiseMin{-14};
            // Maximum noise intensity
            int noiseMax{64};
            // Should we blend noise over a region?
            bool softNoise{true};
            // Radius for soft noise (in pixels) around the chosen point.
            unsigned int spread{6};
            // Controls how quickly the noise intensity falls off.
            float softnessFactor{1.1};
            // Favor stretching noise horizontally
            float directionBiasX{0.2};
            // Favor stretching vertically
            float directionBiasY{0.5};
            // Global noise multiplier
            float scaleStrength{0.5};
            // Frequency of variation in procedural noise
            float frequency{0.9};
            // Adds extra random offset to each point
            float jitter{0.4};
            // Controls intensity ramp (e.g. for sharpening/blooming)
            float noisePower{0.8};
            // Base level without noise
            float baseHeight{0.6};
        };
        fe::Image generateNormalFromPetal(const fe::Image& sourceImage, const NoiseOptions& options) {
            sf::Vector2f sizeF = sourceImage.getSize();
            int width = static_cast<int>(sizeF.x);
            int height = static_cast<int>(sizeF.y);
            fe::Image normalMap;
            normalMap.create(width, height, sf::Color::Transparent);
            auto sample = [&](int sx, int sy) -> float {
                if(sourceImage.getPixel(sx, sy).a != 255){
                    return 0.f;
                }
                float value = options.baseHeight;
                for(unsigned int n = 0; n < options.numPoints; ++n){
                    int jitterX = EvoAI::randomGen().random(-options.jitter, options.jitter);
                    int jitterY = EvoAI::randomGen().random(-options.jitter, options.jitter);
                    int cx = sx + EvoAI::randomGen().random(-static_cast<int>(options.spread), static_cast<int>(options.spread)) + jitterX;
                    int cy = sy + EvoAI::randomGen().random(-static_cast<int>(options.spread), static_cast<int>(options.spread)) + jitterY;
                    if(cx <= 0 || cx >= width - 1 || cy <= 0 || cy >= height - 1){
                        continue;
                    }
                    if(sourceImage.getPixel(cx, cy).a != 255){
                        continue;
                    }
                    float dx = static_cast<float>(sx - cx) * (1.0f + options.directionBiasX);
                    float dy = static_cast<float>(sy - cy) * (1.0f + options.directionBiasY);
                    float dist = std::sqrt(dx * dx + dy * dy);
                    if(dist > static_cast<float>(options.spread)){
                        continue;
                    }
                    float raw = EvoAI::randomGen().random(options.noiseMin, options.noiseMax) / 255.f;
                    float shaped = std::pow(std::abs(raw), options.noisePower) * (raw < 0.f ? -1.f : 1.f);
                    float falloff = options.softNoise ? std::exp(-dist * options.softnessFactor) : 1.0f;
                    value += shaped * falloff * options.scaleStrength;
                }
                return value;
            };
            for(int y = 1; y < height - 1; ++y){
                for(int x = 1; x < width - 1; ++x){
                    if(sourceImage.getPixel(x, y).a != 255){
                        continue;
                    }
                    float hL = sample(x - 1, y);
                    float hR = sample(x + 1, y);
                    float hU = sample(x, y - 1);
                    float hD = sample(x, y + 1);
                    fe::Vec3f normal(- (hR - hL), - (hD - hU), 1.f);
                    normal /= normal.length();
                    auto r = static_cast<sf::Uint8>((normal.x * 0.5f + 0.5f) * 255);
                    auto g = static_cast<sf::Uint8>((normal.y * 0.5f + 0.5f) * 255);
                    auto b = static_cast<sf::Uint8>((normal.z * 0.5f + 0.5f) * 255);
                    normalMap.setPixel(x, y, sf::Color(r, g, b));
                }
            }
            return normalMap;
        }
        struct EmissiveOptions{
            // Distances >= this will produce no emission.
            float colorThreshold{0.2f};
            // How bright the worst-matching pixel still glows [0…1].
            float minIntensity{0.1f};
            // How bright the perfect-match pixel glows [0…1].
            float maxIntensity{1.2f};
            // Shape curve for the match factor: 1=linear, >1=sharpen, <1=smooth.
            float falloffPower{1.0f};
        };
        float computeColorDistance(const sf::Color& a, const sf::Color& b) noexcept{
            float dr = (b.r - a.r) * (1.0f/255.f);
            float dg = (b.g - a.g) * (1.0f/255.f);
            float db = (b.b - a.b) * (1.0f/255.f);
            return std::sqrt(dr * dr + dg * dg + db * db);
        }
        float computeIntensity(const sf::Color& a, const sf::Color& b, const EmissiveOptions& opt) noexcept{
            auto dist = computeColorDistance(a, b);
            float invThresh = 1.0f / std::max(opt.colorThreshold, 1e-6f);
            float rawMatch = 1.0f - std::clamp(dist * invThresh, 0.f, 1.f);
            float match = std::pow(rawMatch, opt.falloffPower);
            return (opt.minIntensity + (opt.maxIntensity - opt.minIntensity) * match);
        }
        fe::Image generateEmissiveFromPetal(const fe::Image& sourceImage, const EmissiveOptions& opt){
            auto size = sourceImage.getSize();
            int W = static_cast<int>(size.x);
            int H = static_cast<int>(size.y);
            int cx = W/2, cy = H/2;
            sf::Color centerColor = sourceImage.getPixel(cx, cy);
            fe::Image emissive;
            emissive.create(W, H, sf::Color::Black);
            for(int y = 0; y < H; ++y){
                for(int x = 0; x < W; ++x){
                    auto srcColor = sourceImage.getPixel(x, y);
                    const bool isTransparent = srcColor.a != 255;
                    if(isTransparent){
                        continue;
                    }
                    auto intensity = computeIntensity(centerColor, srcColor, opt);
                    if(intensity < opt.colorThreshold){
                        continue;
                    }
                    emissive.setPixel(x, y, srcColor);
                }
            }
            return emissive;
        }
    } // namespace priv
    bool generateEllipticalCylinderSegment(
        fe::gltf::Mesh& meshPart,
        const fe::Vec3f& bottomCenter, const fe::Vec3f& topCenter,
        float bottomRadiusX, float bottomRadiusZ, float topRadiusX, float topRadiusZ, int radialSegments,
        bool addBottomCap, bool addTopCap, bool generateUVs,
        std::optional<fe::Vec3f> uVecOverride){

        if(radialSegments < 3 || bottomRadiusX < 0.0f || bottomRadiusZ < 0.0f || topRadiusX < 0.0f || topRadiusZ < 0.0f){
            return false;
        }

        fe::Vec3f axis = topCenter - bottomCenter;
        float height = axis.length();
        if(height < 1e-6f){
            return false;
        }
        axis.normalize();

        fe::Vec3f u_vec, v_vec;
        if(uVecOverride && uVecOverride->lengthSquared() > 1e-6f){
            u_vec = *uVecOverride;
            // Ensure u_vec is orthogonal to axis and normalize
            u_vec = u_vec - axis * axis.dot(u_vec);
            if(u_vec.lengthSquared() < 1e-6f){ // uVecOverride was parallel to axis
                 // Fallback to default u_vec calculation
                if(std::abs(axis.x) > 0.9f || std::abs(axis.y) > 0.9f) {
                    u_vec = fe::Vec3f(0.0f, 0.0f, 1.0f);
                }else{
                    u_vec = fe::Vec3f(1.0f, 0.0f, 0.0f);
                }
                u_vec = u_vec - axis * axis.dot(u_vec); // Make orthogonal
            }
            u_vec.normalize();
        }else{
            // Default u_vec calculation (Gram-Schmidt)
            if(std::abs(axis.x) > 0.9f || std::abs(axis.y) > 0.9f){
                u_vec = fe::Vec3f(0.0f, 0.0f, 1.0f);
            }else{
                u_vec = fe::Vec3f(1.0f, 0.0f, 0.0f);
            }
            if(std::abs(axis.dot(u_vec)) > 0.99f){
                u_vec = fe::Vec3f(0.0f, 1.0f, 0.0f);
            }
            u_vec = u_vec - axis * axis.dot(u_vec);
            u_vec.normalize();
        }
        // Already normalized if axis and u_vec are unit and orthogonal
        v_vec = axis.cross(u_vec);
        std::vector<unsigned int> bottomRingVertexIndices; 
        bottomRingVertexIndices.reserve(radialSegments);
        std::vector<unsigned int> topRingVertexIndices;
        topRingVertexIndices.reserve(radialSegments);
        // Bottom Ring Vertices
        for(int i = 0; i < radialSegments; ++i){
            float angle = static_cast<float>(i) / static_cast<float>(radialSegments) * 2.0f * static_cast<float>(fe::Math::PI);
            float cos_a = std::cos(angle);
            float sin_a = std::sin(angle);

            fe::Vec3f ring_offset_on_plane = u_vec * cos_a + v_vec * sin_a; // Unit vector on the plane
            fe::Vec3f scaled_ring_offset = u_vec * (cos_a * bottomRadiusX) + v_vec * (sin_a * bottomRadiusZ);
            fe::Vec3f position = bottomCenter + scaled_ring_offset;

            fe::Vec3f normal = u_vec * (cos_a / bottomRadiusX) + v_vec * (sin_a / bottomRadiusZ);
            if(bottomRadiusX < 1e-6f || bottomRadiusZ < 1e-6f){
                normal = ring_offset_on_plane; // Fallback for degenerate ellipse
            }
            normal.normalize();
            std::optional<fe::Vec2f> uv;
            if(generateUVs){
                uv = fe::Vec2f{static_cast<float>(i) / static_cast<float>(radialSegments), 0.0f};
            }
            bottomRingVertexIndices.push_back(meshPart.addVertex(fe::gltf::Vertex(position, normal, uv)));
        }
        // Top Ring Vertices
        for(int i = 0; i < radialSegments; ++i){
            float angle = static_cast<float>(i) / static_cast<float>(radialSegments) * 2.0f * static_cast<float>(fe::Math::PI);
            float cos_a = std::cos(angle);
            float sin_a = std::sin(angle);

            fe::Vec3f ring_offset_on_plane = u_vec * cos_a + v_vec * sin_a;
            fe::Vec3f scaled_ring_offset = u_vec * (cos_a * topRadiusX) + v_vec * (sin_a * topRadiusZ);
            fe::Vec3f position = topCenter + scaled_ring_offset;
            
            fe::Vec3f normal = u_vec * (cos_a / topRadiusX) + v_vec * (sin_a / topRadiusZ);
            if(topRadiusX < 1e-6f || topRadiusZ < 1e-6f){
                normal = ring_offset_on_plane;
            }
            normal.normalize();
            std::optional<fe::Vec2f> uv;
            if(generateUVs){
                uv = fe::Vec2f{static_cast<float>(i) / static_cast<float>(radialSegments), 1.0f};
            }
            topRingVertexIndices.push_back(meshPart.addVertex(fe::gltf::Vertex(position, normal, uv)));
        }

        // Side Wall Triangles
        for(int i = 0; i < radialSegments; ++i){
            unsigned int b_curr = bottomRingVertexIndices[i];
            unsigned int b_next = bottomRingVertexIndices[(i + 1) % radialSegments];
            unsigned int t_curr = topRingVertexIndices[i];
            unsigned int t_next = topRingVertexIndices[(i + 1) % radialSegments];
            meshPart.addTriangle(b_curr, t_curr, t_next);
            meshPart.addTriangle(b_curr, t_next, b_next);
        }

        // Caps (Simplified UVs for caps, normals along axis)
        if(addBottomCap && (bottomRadiusX > 1e-6f || bottomRadiusZ > 1e-6f)){
            fe::Vec3f capNormal = axis * -1.0f;
            unsigned int bottomCenterVtxIdx = meshPart.addVertex(
                fe::gltf::Vertex(bottomCenter, capNormal, generateUVs ? std::make_optional(fe::Vec2f{0.5f, 0.5f}) : std::nullopt)
            );
            std::vector<unsigned int> bottomCapRingIndices;
            bottomCapRingIndices.reserve(radialSegments);
            for(int i = 0; i < radialSegments; ++i){
                const fe::gltf::Vertex& sideVertex = meshPart.vertices[bottomRingVertexIndices[i]];
                std::optional<fe::Vec2f> uv;
                if(generateUVs){
                    float angle = static_cast<float>(i) / static_cast<float>(radialSegments) * 2.0f * static_cast<float>(fe::Math::PI);
                    uv = fe::Vec2f{0.5f + 0.5f * std::cos(angle), 0.5f + 0.5f * std::sin(angle)};
                }
                bottomCapRingIndices.push_back(meshPart.addVertex(fe::gltf::Vertex(sideVertex.position, capNormal, uv)));
            }
            for(int i = 0; i < radialSegments; ++i){
                meshPart.addTriangle(bottomCenterVtxIdx, bottomCapRingIndices[(i + 1) % radialSegments], bottomCapRingIndices[i]);
            }
        }

        if(addTopCap && (topRadiusX > 1e-6f || topRadiusZ > 1e-6f)){
            fe::Vec3f capNormal = axis;
            unsigned int topCenterVtxIdx = meshPart.addVertex(
                fe::gltf::Vertex(topCenter, capNormal, generateUVs ? std::make_optional(fe::Vec2f{0.5f, 0.5f}) : std::nullopt)
            );
            std::vector<unsigned int> topCapRingIndices; 
            topCapRingIndices.reserve(radialSegments);
            for(int i = 0; i < radialSegments; ++i){
                const fe::gltf::Vertex& sideVertex = meshPart.vertices[topRingVertexIndices[i]];
                std::optional<fe::Vec2f> uv;
                if(generateUVs){
                    float angle = static_cast<float>(i) / static_cast<float>(radialSegments) * 2.0f * static_cast<float>(fe::Math::PI);
                    uv = fe::Vec2f{0.5f + 0.5f * std::cos(angle), 0.5f + 0.5f * std::sin(angle)};
                }
                topCapRingIndices.push_back(meshPart.addVertex(fe::gltf::Vertex(sideVertex.position, capNormal, uv)));
            }
            for(int i = 0; i < radialSegments; ++i){
                meshPart.addTriangle(topCenterVtxIdx, topCapRingIndices[i], topCapRingIndices[(i + 1) % radialSegments]);
            }
        }
        if(!meshPart.vertices.empty()){
            meshPart.hasNormals = true;
            if(generateUVs){
                meshPart.hasTexCoords0 = true;
            }
        }
        return true;
    }
    bool generateSegmentedCylinder(fe::gltf::Mesh& meshPart, const std::vector<CylinderSegment>& profilePoints,
                                        int radialSegments, bool addBaseCap, bool addTipCap, bool generateUVs){
        if(profilePoints.size() < 2){
            return false;
        }
        // Determine a consistent u_vec for the whole filament if not specified per node.
        // This prevents twisting unless intended.
        // Calculate from the first segment, or allow an override.
        std::optional<fe::Vec3f> filament_uVec_orientation = std::nullopt;
        if(profilePoints.size() >=2){
            fe::Vec3f first_axis = profilePoints[1].center - profilePoints[0].center;
            if(first_axis.lengthSquared() > 1e-6f){
                first_axis.normalize();
                fe::Vec3f temp_u_vec;
                if(std::abs(first_axis.x) > 0.9f || std::abs(first_axis.y) > 0.9f){
                    temp_u_vec = fe::Vec3f(0.0f, 0.0f, 1.0f);
                }else{
                    temp_u_vec = fe::Vec3f(1.0f, 0.0f, 0.0f);
                }
                if(std::abs(first_axis.dot(temp_u_vec)) > 0.99f){
                    temp_u_vec = fe::Vec3f(0.0f, 1.0f, 0.0f);
                }
                temp_u_vec = temp_u_vec - first_axis * first_axis.dot(temp_u_vec);
                temp_u_vec.normalize();
                filament_uVec_orientation = temp_u_vec;
            }
        }
        for(std::size_t i = 0; i < profilePoints.size() - 1; ++i){
            const auto& node_bottom = profilePoints[i];
            const auto& node_top = profilePoints[i + 1];

            bool cap_bottom_this_segment = (i == 0) && addBaseCap;
            bool cap_top_this_segment = (i == profilePoints.size() - 2) && addTipCap;

            // Allow per-node uVec orientation if defined in CylinderSegment, else use consistent one
            std::optional<fe::Vec3f> current_uVec = node_bottom.uVecOrientation ? node_bottom.uVecOrientation : filament_uVec_orientation;
            generateEllipticalCylinderSegment(
                            meshPart,
                            node_bottom.center, node_top.center,
                            node_bottom.radiusX, node_bottom.radiusZ,
                            node_top.radiusX, node_top.radiusZ,
                            radialSegments,
                            cap_bottom_this_segment, cap_top_this_segment,
                            generateUVs,
                            current_uVec
            );
        }
        return true;
    }
    bool generateStigma(
        fe::gltf::Mesh& meshPart,
        const fe::CylinderSegment& filamentTopNode,
        const fe::FlowerParameters& params, int radialSegments, bool generateUVs){

        float stigmaHeight = params.pistilStigmaHeight;
        if(stigmaHeight < 0.001f){
            stigmaHeight = 0.001f;
        }
        // Stigma widening factor relative to the filament's top radius
        float stigmaMaxWidthFactor = params.pistilStigmaMaxWidthFactor;
        float stigmaTipNarrowFactor = params.pistilStigmaTipNarrowFactor;

        // Radii at the base of the stigma (connection point)
        float baseR_X = filamentTopNode.radiusX;
        float baseR_Z = filamentTopNode.radiusZ;

        // Radii at the widest part of the stigma
        float midR_X = baseR_X * stigmaMaxWidthFactor;
        float midR_Z = baseR_Z * stigmaMaxWidthFactor;

        // Radii at the tip of the stigma
        float tipR_X = midR_X * stigmaTipNarrowFactor;
        float tipR_Z = midR_Z * stigmaTipNarrowFactor;

        std::vector<fe::CylinderSegment> stigmaShapeProfile;
        stigmaShapeProfile.reserve(3);
        // Node 0: Base of stigma (at the top of the filament)
        stigmaShapeProfile.push_back(
            {
                filamentTopNode.center, // Use the center from the filament's top node
                baseR_X,
                baseR_Z,
                std::nullopt
            }
        );
        // Node 1: Widest point
        float y_mid = filamentTopNode.center.y + stigmaHeight * 0.40f;
        stigmaShapeProfile.push_back(
            {
                {filamentTopNode.center.x, y_mid, filamentTopNode.center.z},
                midR_X,
                midR_Z,
                std::nullopt
            }
        );

        // Node 2: Tip of stigma (small cap)
        float y_top = filamentTopNode.center.y + stigmaHeight;
        stigmaShapeProfile.push_back(
            {
                {filamentTopNode.center.x, y_top, filamentTopNode.center.z},
                tipR_X,
                tipR_Z,
                std::nullopt
            }
        );
        return fe::generateSegmentedCylinder(
            meshPart,
            stigmaShapeProfile,
            radialSegments,
            false,
            true,
            generateUVs
        );
    }
    void generatePistil(fe::gltf::Scene& scene, const fe::Vec3f& position, const fe::FlowerParameters& params, 
                        int pistil_filament_mat_idx, int pistil_stigma_max_idx, int pistilID){
        fe::gltf::Mesh& pistilMesh = scene.createMeshPart("Pistil_Style_Mesh_" + std::to_string(pistilID));
        auto pistilFilamentNodeIndex = scene.addNode(fe::gltf::Node::makeNode("Pistil_Style_Node_" + std::to_string(pistilID), pistilMesh));
        pistilMesh.materialIndex = pistil_filament_mat_idx;
        pistilMesh.hasNormals = true;
        pistilMesh.hasTexCoords0 = false;
        std::vector<fe::CylinderSegment> filamentProfile;
        filamentProfile.reserve(5);
        float stemTopY = position.y;
        float pistilTotalFilamentHeight = params.pistilStyleHeight;
        filamentProfile.push_back(
            {
                {0.0f, stemTopY, 0.0f},
                params.pistilStyleRadius * 1.0f,
                params.pistilStyleRadius * 1.0f,
                std::nullopt
            }
        );
        filamentProfile.push_back(
            {
                {0.0f, stemTopY + pistilTotalFilamentHeight * 0.20f, 0.0f}, 
                params.pistilStyleRadius * 1.0f,
                params.pistilStyleRadius * 1.0f,
                std::nullopt
            }
        );
        filamentProfile.push_back(
            {
                {0.0f, stemTopY + pistilTotalFilamentHeight * 0.50f, 0.0f},
                params.pistilStyleRadius * 0.8f,
                params.pistilStyleRadius * 0.8f,
                std::nullopt
            }
        );
        filamentProfile.push_back(
            {
                {0.0f, stemTopY + pistilTotalFilamentHeight * 0.70f, 0.0f},
                params.pistilStyleRadius * 0.70f,
                params.pistilStyleRadius * 0.70f,
                std::nullopt
            }
        );
        filamentProfile.push_back(
            {
                {0.0f, stemTopY + pistilTotalFilamentHeight, 0.0f},
                params.pistilStyleRadius * 0.60f,
                params.pistilStyleRadius * 0.60f,
                std::nullopt
            }
        );
        // Generate the pistil filament
        fe::generateSegmentedCylinder(
            pistilMesh,
            filamentProfile,
            params.pistilStigmaRadialSegments,
            true,
            false,
            false
        );

        fe::gltf::Mesh& stigmaMesh = scene.createMeshPart("Pistil_Stigma_Mesh_" + std::to_string(pistilID));
        stigmaMesh.materialIndex = pistil_stigma_max_idx;
        auto stigmaNodeIndex = scene.addNode(fe::gltf::Node::makeNode("Stigma_Node_" + std::to_string(pistilID), stigmaMesh));
        scene.addNode(fe::gltf::Node::makeGroup("Pistil_Group_Node_" + std::to_string(pistilID), {pistilFilamentNodeIndex, stigmaNodeIndex}));
        // The top node of the filament IS the base for the stigma
        const fe::CylinderSegment& filament_tip_node = filamentProfile.back();
        bool stigmaGenerateUVs = true;
        fe::generateStigma(
            stigmaMesh,
            filament_tip_node,
            params,
            params.pistilStigmaRadialSegments,
            stigmaGenerateUVs
        );
    }
	void generateStamen(fe::gltf::Scene& scene, const fe::Vec3f& position, const fe::FlowerParameters& params, 
                        int stamen_filament_mat_idx, int stamen_anther_mat_idx, int stamenID){
		fe::gltf::Mesh& stamenFilamentMesh = scene.createMeshPart("Stamen_Filament_Mesh_" + std::to_string(stamenID));
        auto stamenFilamentNodeIndex = scene.addNode(fe::gltf::Node::makeNode("Stamen_Filament_Node_" + std::to_string(stamenID), stamenFilamentMesh));
		stamenFilamentMesh.materialIndex = stamen_filament_mat_idx;
        stamenFilamentMesh.hasNormals = true;
        stamenFilamentMesh.hasTexCoords0 = false;
		std::vector<fe::CylinderSegment> filamentProfile;
		filamentProfile.reserve(3);
		float stemTopY = position.y - 0.01f;
		float stamenFilamentHeight = params.stamenFilamentHeight;
		filamentProfile.push_back(
            {
                {position.x, stemTopY, position.z},
                params.stamenFilamentRadius * 1.0f,
                params.stamenFilamentRadius * 1.0f,
                std::nullopt
            }
		);
		filamentProfile.push_back(
            {
                {position.x, stemTopY + stamenFilamentHeight * 0.20f, position.z},
                params.stamenFilamentRadius * 0.5f,
                params.stamenFilamentRadius * 0.5f,
                std::nullopt
            }
		);
		filamentProfile.push_back(
            {
                {position.x, stemTopY + stamenFilamentHeight, position.z},
                params.stamenFilamentRadius * 0.15f,
                params.stamenFilamentRadius * 0.15f,
                std::nullopt
            }
		);
		// Generate the stamen filament
		fe::generateSegmentedCylinder(
			stamenFilamentMesh,
			filamentProfile,
			params.stamenFilamentRadialSegments,
			true,
			true,
			false
		);
		fe::gltf::Mesh& StamenAntherMesh = scene.createMeshPart("Stamen_Anther_Mesh_" + std::to_string(stamenID));
		StamenAntherMesh.materialIndex = stamen_anther_mat_idx;
        auto stamenAntherNodeIndex = scene.addNode(fe::gltf::Node::makeNode("Stamen_Anther_Node_" + std::to_string(stamenID), StamenAntherMesh));
        scene.addNode(fe::gltf::Node::makeGroup("Stamen_Group_Node_" + std::to_string(stamenID), {stamenFilamentNodeIndex, stamenAntherNodeIndex}));
		float stamenFilamentBase = (stemTopY + stamenFilamentHeight) - 0.001f;
		std::vector<fe::CylinderSegment> antherProfile;
		antherProfile.reserve(6);
		antherProfile.push_back(
            {
                {position.x, stamenFilamentBase, position.z},
                params.stamenFilamentRadius * 0.1f,
                params.stamenFilamentRadius * 0.1f,
                std::nullopt
            }
		);
		antherProfile.push_back(
            {
                {position.x, stamenFilamentBase + params.stamenAntherHeight * 0.2f, position.z},
                params.stamenFilamentRadius * 0.2f,
                params.stamenFilamentRadius * 0.2f,
                std::nullopt
            }
		);
		antherProfile.push_back(
            {
                {position.x, stamenFilamentBase + params.stamenAntherHeight * 0.4f, position.z},
                params.stamenFilamentRadius * 0.4f,
                params.stamenFilamentRadius * 0.4f,
                std::nullopt
            }
		);
		antherProfile.push_back(
            {
                {position.x, stamenFilamentBase + params.stamenAntherHeight * 0.6f, position.z},
                params.stamenFilamentRadius * 0.6f,
                params.stamenFilamentRadius * 0.6f,
                std::nullopt
            }
		);
		antherProfile.push_back(
            {
                {position.x, stamenFilamentBase + params.stamenAntherHeight * 0.8f, position.z},
                params.stamenFilamentRadius * 0.4f,
                params.stamenFilamentRadius * 0.4f,
                std::nullopt
            }
		);
		antherProfile.push_back(
            {
                {position.x, stamenFilamentBase + params.stamenAntherHeight, position.z},
                params.stamenFilamentRadius * 0.01f,
                params.stamenFilamentRadius * 0.01f,
                std::nullopt
            }
		);
		fe::generateSegmentedCylinder(
			StamenAntherMesh,
			antherProfile,
			params.stamenAntherRadialSegments,
			true,
			true,
			true
		);
	}
    void generateStem(fe::gltf::Scene& scene, const fe::FlowerParameters& params, int stemMaterialIndex){
        if(params.stemSegments < 3){
            return;
        }
        std::string meshPartName = "Stem";
        fe::gltf::Mesh& stemMesh = scene.createMeshPart(meshPartName);
        scene.addNode(fe::gltf::Node::makeNode("Stem_Node", stemMesh));
        stemMesh.materialIndex = stemMaterialIndex;
        stemMesh.hasNormals = true;
        stemMesh.hasTexCoords0 = false;
        // Stem grows upwards from Y=0
        const float height = params.stemHeight;
        const float radius = params.stemRadius;
        const int segments = params.stemSegments;
        fe::Vec3f bottom = {0.0f, 0.0f, 0.0f};
        fe::Vec3f top = {0.0f, height, 0.0f};
		std::vector<fe::CylinderSegment> profiles;
        profiles.reserve(2);
		profiles.push_back({bottom, radius, radius, std::nullopt});
		profiles.push_back({top, radius, radius, std::nullopt});
        generateSegmentedCylinder(stemMesh, profiles, segments, true, true, false);
    }
    void generatePetalLayer(
        fe::gltf::Scene& scene,
        const std::vector<fe::Vec2i>& simplifiedContour,
        const fe::Image& petalLayerTexture, int layerIndex,
        const fe::Vec3f& position, const fe::FlowerParameters& params){
        const auto numPoints = simplifiedContour.size();
        const auto imgWidth = petalLayerTexture.getSize().x;
        const auto imgHeight = petalLayerTexture.getSize().y;
        // Need at least 3 points for a polygon and valid dimensions
        if(numPoints < 3 || imgWidth <= 0 || imgHeight <= 0){
            return;
        }
        // Create Texture and Material for this Petal Layer
        std::string textureName = "Petal_Layer_Texture_" + std::to_string(layerIndex);
        int textureIndex = scene.addTexture(fe::gltf::TextureInfo::createFromImage(textureName, petalLayerTexture));
        std::string materialName = "Petal_Layer_Material_" + std::to_string(layerIndex);
        int normalIndex = -1;
        int emissiveIndex = -1;
        if(params.useNormals){
            std::string normalName = "Petal_Layer_Normal_" + std::to_string(layerIndex);
            auto noiseImage = priv::generateNormalFromPetal(petalLayerTexture, 
                {
                    .numPoints = 32,
                    .noiseMin = -14, 
                    .noiseMax = 64, 
                    .softNoise = true, 
                    .spread = 6, 
                    .softnessFactor = 1.1,
                    .directionBiasX = 0.2,
                    .directionBiasY = 0.5,
                    .scaleStrength = 0.5,
                    .frequency = 0.9,
                    .jitter = 0.4,
                    .noisePower = 0.8,
                    .baseHeight = 0.8,
                }
            );
            auto normTex = fe::gltf::TextureInfo::createFromImage(normalName, noiseImage);
            normalIndex = scene.addTexture(normTex);
        }
        std::string meshPartName = "Petal_Layer_Mesh_" + std::to_string(layerIndex);
        auto& petalMesh = scene.createMeshPart(meshPartName);
        auto petalNode = fe::gltf::Node::makeNode("Petal_Layer_Node_" + std::to_string(layerIndex), petalMesh);
        // Use double for precision in geometric calculations
        const double d_petalScaleFactor = static_cast<double>(params.petalScaleFactor);
        const double d_connectionRadiusPx = static_cast<double>(params.connectionRadiusPx);
        const double d_droopStartRadiusPx = static_cast<double>(params.droopStartRadiusPx);
        const double d_peakHeightOffset = static_cast<double>(params.peakHeightOffset);
        const double d_connectionVerticalOffset = static_cast<double>(params.connectionVerticalOffset);
        const double d_petalDroopFactor = static_cast<double>(params.petalDroopFactor);
        const double d_base_y = static_cast<double>(position.y);
        if(params.useEmissive){
            auto opts = priv::EmissiveOptions{
                .colorThreshold = 0.8f,
                .minIntensity = 0.01f,
                .maxIntensity = 1.5f,
                .falloffPower = 1.2f
            };
            auto emissiveImage = priv::generateEmissiveFromPetal(petalLayerTexture, opts);
            auto emissiveTex = fe::gltf::TextureInfo::createFromImage("Petal_Layer_Emissive_" + std::to_string(layerIndex), emissiveImage);
            emissiveIndex = scene.addTexture(emissiveTex);
            JsonBox::Object extra;
            JsonBox::Array lights;
            float cx = imgWidth * 0.5f;
            float cz = imgHeight * 0.5f;
            float radius = cx * 1.5f;
            std::array<fe::Vec3f, 1> positions{};
            positions[0] = { cx, 0.0f, cz };
            auto to3DPosition = [&](const fe::Vec3f& position, std::size_t width, std::size_t height){
                const double peak_y = (d_base_y + 0.0025 + d_connectionVerticalOffset) + d_peakHeightOffset;
                double drop_amount = 0.0;
                double normRadius = (radius * d_petalScaleFactor) - d_droopStartRadiusPx;
                if(position.y > 0.0){
                    drop_amount = (normRadius) * d_petalDroopFactor * d_petalScaleFactor;
                }
                const double angle = std::atan2(-cz, cx);
                const double cos_angle = std::cos(angle);
                const double sin_angle = std::sin(angle);
                const double normX = position.x / width - 0.5f;
                const double normZ = position.z / height - 0.5f;
                return fe::Vec3f(normX * cos_angle, 
                                peak_y - drop_amount, 
                                -(normZ * sin_angle));
            };
            auto centerColor = petalLayerTexture.getPixel(static_cast<std::size_t>(positions[0].x), static_cast<std::size_t>(positions[0].z));
            for(const auto& pos : positions){
                auto srcColor = petalLayerTexture.getPixel(pos.x, pos.z);
                if(srcColor == sf::Color::Black || srcColor.a == 0){
                    continue;
                }
                auto intensity = priv::computeIntensity(centerColor, srcColor, opts);
                if(intensity < 1e-2f){
                    continue;
                }
                JsonBox::Object light;
                light["type"]      = "PointLight";
                light["color"]     = static_cast<int>((srcColor.r << 16) | (srcColor.g << 8) | srcColor.b);
                light["intensity"] = intensity / 100.0;
                auto uv = to3DPosition(pos, imgWidth, imgHeight);
                light["position"]  = JsonBox::Array{ uv.x, uv.y, uv.z};
                light["radius"]    = radius * d_petalScaleFactor;
                light["decay"]     = 2.0;
                lights.push_back(light);
            }
            extra["lights"] = lights;
            petalNode.extra = extra;
            scene.addNode(petalNode);
        }else{
            scene.addNode(petalNode);
        }
        auto petalMaterial = fe::gltf::Material::createPetalMaterial(materialName, textureIndex, normalIndex, emissiveIndex);
        int materialIndexForThisPetalLayer = scene.addMaterial(petalMaterial);
        petalMesh.materialIndex = materialIndexForThisPetalLayer;
        const double inner_structural_radius_3d = d_connectionRadiusPx * d_petalScaleFactor;
        double peak_structural_radius_3d = d_droopStartRadiusPx * d_petalScaleFactor;
        if(peak_structural_radius_3d <= inner_structural_radius_3d){
            peak_structural_radius_3d = inner_structural_radius_3d * 1.1;
            if(peak_structural_radius_3d < 1e-15){
                peak_structural_radius_3d = 0.01;
            }
        }
        const double attachment_y = d_base_y + d_connectionVerticalOffset;
        const double peak_y = attachment_y + d_peakHeightOffset;
        std::vector<fe::gltf::Vertex> inner_ring_vertices_temp;
        inner_ring_vertices_temp.reserve(numPoints);
        std::vector<fe::gltf::Vertex> peak_ring_vertices_temp;
        peak_ring_vertices_temp.reserve(numPoints);
        std::vector<fe::gltf::Vertex> contour_ring_vertices_temp;
        contour_ring_vertices_temp.reserve(numPoints);
        const double d_imgWidth = static_cast<double>(imgWidth);
        const double d_imgHeight = static_cast<double>(imgHeight);
        const double center_x_px = d_imgWidth / 2.0;
        const double center_y_px = d_imgHeight / 2.0;
        for(const fe::Vec2i& point : simplifiedContour){
            const double px = static_cast<double>(point.x);
            const double py = static_cast<double>(point.y);

            const double dx = px - center_x_px;
            const double dy = py - center_y_px;
            const double contour_dist_px_sq = dx * dx + dy * dy;
            const double contour_dist_px = (contour_dist_px_sq > 1e-15) ? std::sqrt(contour_dist_px_sq) : 0.0;
            const double angle = (contour_dist_px > 1e-15) ? std::atan2(-dy, dx) : 0.0;
            const double cos_angle = std::cos(angle);
            const double sin_angle = std::sin(angle);

            fe::gltf::Vertex currentVertex;
            // Outer Contour UV (C)
            // V direct from py (top of image is V=0)
            fe::Vec2f vt_contour(
                std::clamp(static_cast<float>((px + 0.5f) / d_imgWidth), 0.0f, 1.0f),
                std::clamp(static_cast<float>((py + 0.5f) / d_imgHeight), 0.0f, 1.0f)
            );
            vt_contour.x = std::clamp(vt_contour.x, 0.0f, 1.0f);
            vt_contour.y = std::clamp(vt_contour.y, 0.0f, 1.0f);
            // Peak Ring UV (B)
            float u_peak_factor = params.droopStartRadiusPx / imgWidth;
            float v_peak_factor = params.droopStartRadiusPx / imgHeight;
            // Use -sin_angle for V downward from UV center
            fe::Vec2f vt_peak(
                std::clamp(0.5f + u_peak_factor * static_cast<float>(cos_angle), 0.0f, 1.0f),
                std::clamp(0.5f + v_peak_factor * static_cast<float>(-sin_angle), 0.0f, 1.0f)
            );
            // Inner Ring UV (A)
            float u_inner_factor = params.connectionRadiusPx / imgWidth;
            float v_inner_factor = params.connectionRadiusPx / imgHeight;
            // Use -sin_angle for V downward from UV center
            fe::Vec2f vt_inner(
                std::clamp(0.5f + u_inner_factor * static_cast<float>(cos_angle), 0.0f, 1.0f),
                std::clamp(0.5f + v_inner_factor * static_cast<float>(-sin_angle), 0.0f, 1.0f)
            );
            // Inner Ring Vertex (A)
            currentVertex.position = {
                static_cast<float>(inner_structural_radius_3d * cos_angle),
                static_cast<float>(attachment_y),
                static_cast<float>(inner_structural_radius_3d * sin_angle)
            };
            currentVertex.texCoord0 = vt_inner;
            fe::Vec3f n_inner_val = {static_cast<float>(cos_angle) * 0.1f, 1.0f, static_cast<float>(sin_angle) * 0.1f};
            currentVertex.normal = n_inner_val;
            currentVertex.normal->normalize();
            inner_ring_vertices_temp.push_back(currentVertex);
            // Peak Ring Vertex (B)
            currentVertex.position = {
                static_cast<float>(peak_structural_radius_3d * cos_angle),
                static_cast<float>(peak_y),
                static_cast<float>(peak_structural_radius_3d * sin_angle)
            };
            currentVertex.texCoord0 = vt_peak;
            fe::Vec3f n_peak_val = {static_cast<float>(cos_angle) * 0.3f, 0.9f, static_cast<float>(sin_angle) * 0.3f};
            currentVertex.normal = n_peak_val;
            currentVertex.normal->normalize();
            peak_ring_vertices_temp.push_back(currentVertex);
            // Contour Ring Vertex (C)
            const double contour_dist_3d = contour_dist_px * d_petalScaleFactor;
            const double pixel_dist_beyond_droop_start = std::max(0.0, contour_dist_px - d_droopStartRadiusPx);
            const double droop_amount = pixel_dist_beyond_droop_start * d_petalDroopFactor * d_petalScaleFactor;
            currentVertex.position = {
                static_cast<float>(contour_dist_3d * cos_angle),
                static_cast<float>(peak_y - droop_amount),
                static_cast<float>(contour_dist_3d * sin_angle)
            };
            currentVertex.texCoord0 = vt_contour;
            fe::Vec3f n_contour_val = {static_cast<float>(cos_angle), 0.5f - static_cast<float>(droop_amount) * 0.5f, static_cast<float>(sin_angle)};
            currentVertex.normal = n_contour_val;
            currentVertex.normal->normalize();
            contour_ring_vertices_temp.push_back(currentVertex);
        }
        // Add all generated vertices to the fe::gltf::Mesh vertex buffer.
        unsigned int base_idx_inner = 0;
        for(const auto& v : inner_ring_vertices_temp) {
            petalMesh.addVertex(v);
        }
        unsigned int base_idx_peak = static_cast<unsigned int>(numPoints);
        for(const auto& v : peak_ring_vertices_temp){
            petalMesh.addVertex(v);
        }
        unsigned int base_idx_contour = static_cast<unsigned int>(numPoints * 2);
        for(const auto& v : contour_ring_vertices_temp){
            petalMesh.addVertex(v);
        }
        // Triangulate the Two Strips
        for(std::size_t i = 0; i < numPoints; ++i){
            std::size_t next_i = (i + 1) % numPoints;
            // Indices for the 4 corners of the inner->peak strip quad
            unsigned int idx_in1 = base_idx_inner + static_cast<unsigned int>(i);
            unsigned int idx_in2 = base_idx_inner + static_cast<unsigned int>(next_i);
            unsigned int idx_pk1 = base_idx_peak + static_cast<unsigned int>(i);
            unsigned int idx_pk2 = base_idx_peak + static_cast<unsigned int>(next_i);
            // Triangle 1a: in1, pk1, pk2 (CCW winding for glTF default)
            petalMesh.addTriangle(idx_in1, idx_pk1, idx_pk2);
            // Triangle 1b: in1, pk2, in2
            petalMesh.addTriangle(idx_in1, idx_pk2, idx_in2);
            // Indices for the 4 corners of the peak->contour strip quad
            unsigned int idx_co1 = base_idx_contour + static_cast<unsigned int>(i);
            unsigned int idx_co2 = base_idx_contour + static_cast<unsigned int>(next_i);
            // idx_pk1, idx_pk2 are reused
            // Triangle 2a: pk1, co1, co2
            petalMesh.addTriangle(idx_pk1, idx_co1, idx_co2);
            // Triangle 2b: pk1, co2, pk2
            petalMesh.addTriangle(idx_pk1, idx_co2, idx_pk2);
        }
    }
} // namespace fe
