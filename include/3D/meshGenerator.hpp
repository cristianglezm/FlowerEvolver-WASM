#ifndef FLOWER_EVOLVER_3D_GEOMETRY_GENERATOR_HPP
#define FLOWER_EVOLVER_3D_GEOMETRY_GENERATOR_HPP

#include <3D/GLTF.hpp>
#include <3D/FlowerParameters.hpp>

#include <vector>
#include <string>

namespace fe{
    /**
     * @brief Generates a single segment of a cylinder with potentially elliptical and tapered cross-sections.
     * @details Creates the wall and optionally top/bottom caps.
     *          UVs are generated for the wall (V along axis, U wraps). Cap UVs are planar.
     *
     * @param meshPart fe::gltf::Mesh& The mesh part to add geometry to.
     * @param bottomCenter const fe::Vec3f& Center of the bottom elliptical face.
     * @param topCenter const fe::Vec3f& Center of the top elliptical face.
     * @param bottomRadiusX float Radius of the bottom ellipse along its local X-axis (defined by u_vec).
     * @param bottomRadiusZ float Radius of the bottom ellipse along its local Z-axis (defined by v_vec).
     * @param topRadiusX float Radius of the top ellipse along its local X-axis.
     * @param topRadiusZ float Radius of the top ellipse along its local Z-axis.
     * @param radialSegments int Number of segments around the circumference. Must be >= 3.
     * @param addBottomCap bool If true, a bottom cap will be generated.
     * @param addTopCap bool If true, a top cap will be generated.
     * @param generateUVs bool If true, UV coordinates (TEXCOORD_0) will be generated.
     * @param uVecOverride std::optional<fe::Vec3f> Optional: provide a specific u_vec for orientation.
     *                     If nullopt, it will be calculated.
     * @return bool True if geometry was successfully generated.
     */
    bool generateEllipticalCylinderSegment(
        fe::gltf::Mesh& meshPart,
        const fe::Vec3f& bottomCenter, const fe::Vec3f& topCenter,
        float bottomRadiusX, float bottomRadiusZ,
        float topRadiusX, float topRadiusZ, int radialSegments,
        bool addBottomCap = false, bool addTopCap = false, bool generateUVs = true,
        std::optional<fe::Vec3f> uVecOverride = std::nullopt);
    /**
     * @brief storage for filament info.
     */
    struct CylinderSegment final{
        fe::Vec3f center;
        float radiusX;
        float radiusZ;
        std::optional<fe::Vec3f> uVecOrientation;
        CylinderSegment(const fe::Vec3f& cnt, float rdX, float rdZ, std::optional<fe::Vec3f> uVecOrient = std::nullopt) noexcept
        : center{cnt}
        , radiusX{rdX}
        , radiusZ{rdZ}
        , uVecOrientation{uVecOrient}{}
    };
    /**
     * @brief Generates a segmented cylinder.
     * @param meshPart fe::gltf::Mesh& The mesh part to add a segmented cylinder to.
     * @param profilePoints const std::vector<CylinderSegment>& A series of nodes defining the
     *                      center, radii, and orientation along the filament's length.
     *                      Must have at least 2 points to form a segment.
     * @param radialSegments int Number of segments around the circumference.
     * @param addBaseCap bool If true, the very first segment gets a bottom cap.
     * @param addTipCap bool If true, the very last segment gets a top cap.
     * @param generateUVs bool If true, generate UVs.
     * @return bool True if geometry was successfully generated.
     */
    bool generateSegmentedCylinder(
        fe::gltf::Mesh& meshPart, const std::vector<CylinderSegment>& profilePoints,
        int radialSegments = 12, bool addBaseCap = false, bool addTipCap = false, bool generateUVs = false);
    /**
     * @brief Generates the stigma (tip of the pistil) based on the filament's top dimensions.
     *
     * @param meshPart fe::gltf::Mesh& The mesh part to add stigma geometry to.
     * @param filamentTopNode const fe::CylinderSegment& The last node of the pistil filament,
     *                        providing the connection point and initial radii for the stigma.
     * @param params const fe::FlowerParameters& General geometry parameters for reference (e.g., for scaling factors).
     * @param radialSegments int Number of segments around the circumference.
     * @param generateUVs bool If true, generate UV coordinates.
     * @return bool True if geometry was successfully generated.
     */
    bool generateStigma(
        fe::gltf::Mesh& meshPart, const fe::CylinderSegment& filamentTopNode,
        const fe::FlowerParameters& params, int radialSegments, bool generateUVs = false);
    /**
     * @brief Generates a pistil structure and adds it to a GLTF scene.
     * 
     * @param scene A reference to the fe::gltf::Scene data where the generated pistil will be incorporated.
     * @param position A 3D vector that specifies the position within the scene where the pistil should be generated.
     * @param params Geometry parameters that dictate the design details such as shape, size, and resolution of the pistil.
     * @param pistil_filament_mat_idx Material index for the filament, which defines its color and texture properties.
     * @param pistil_stigma_max_idx Material index for the stigma, which determines its visual style.
     * @param pistilID A unique identifier for the pistil instance, useful for tracking or referencing within the scene.
     */
    void generatePistil(fe::gltf::Scene& scene, const fe::Vec3f& position, const fe::FlowerParameters& params, 
                        int pistil_filament_mat_idx, int pistil_stigma_max_idx, int pistilID);
    /**
     * @brief Generates a stamen structure and adds it to a GLTF scene.
     * 
     * @param scene Reference to the fe::gltf::Scene data where the generated stamen will be added.
     * @param position A 3D vector specifying the position in the scene where the stamen should be generated.
     * @param params Geometry parameters that dictate the size, shape, and resolution of the stamen parts.
     * @param stamen_filament_mat_idx Material index for the filament (the stalk supporting the anther),
     *                                determining its color and texture.
     * @param stamen_anther_mat_idx Material index for the anther (the pollen-bearing part of the stamen),
     *                              determining its color and texture.
     * @param stamenID An identifier for the stamen, useful for tracking or referencing this particular instance.
     */
    void generateStamen(fe::gltf::Scene& scene, const fe::Vec3f& position, const fe::FlowerParameters& params, 
                        int stamen_filament_mat_idx, int stamen_anther_mat_idx, int stamenID);
    /**
     * @brief Generates vertices, normals, and triangle indices for a cylindrical stem,
     *        creating a distinct GltfMeshPart for it.
     *
     * Creates geometry for a cylinder oriented along the Y-axis, with closed
     * top and bottom caps. Adds the generated GltfMeshPart to the provided GltfSceneData.
     * Stem geometry typically uses a base color material and does not require texture coordinates.
     *
     * @param scene fe::gltf::Scene& The fe::gltf::Scene object to add the new stem mesh part to.
     * @param params const fe::FlowerParameters& The FlowerParameters struct containing stemHeight, stemRadius, and stemSegments.
     * @param stemMaterialIndex int The index of the pre-defined stem material in scene.materials.
     */
    void generateStem(fe::gltf::Scene& scene, const fe::FlowerParameters& params, int stemMaterialIndex);
    /**
     * @brief Generates vertices, normals, texcoords, and triangle indices for one petal layer,
     *        creating a distinct fe::gltf::Mesh for it with its own texture and material.
     *
     * Creates the geometry based on the simplified contour, fixed inner radii,
     * and other parameters. Adds the generated fe::gltf::Mesh to the provided fe::gltf::Scene.
     *
     * @param scene fe::gltf::Scene& The GltfSceneData object to add the new petal mesh part to.
     * @param simplifiedContour const std::vector<fe::Vec2i>& The vector of 2D points representing the simplified outer shape.
     * @param petalLayerTexture const fe::Image& The raw image data for this specific petal layer's texture.
     * @param layerIndex int The index of the current layer (used for naming).
     * @param position const fe::Vec3f& The position for this layer before offsets.
     * @param params const fe::FlowerParameters& The FlowerParameters struct containing all petal shape and scale parameters.
     */
    void generatePetalLayer(
        fe::gltf::Scene& scene, const std::vector<fe::Vec2i>& simplifiedContour, 
        const fe::Image& petalLayerTexture, int layerIndex,
        const fe::Vec3f& position, const fe::FlowerParameters& params);
} // namespace fe

#endif // FLOWER_EVOLVER_3D_GEOMETRY_GENERATOR_HPP