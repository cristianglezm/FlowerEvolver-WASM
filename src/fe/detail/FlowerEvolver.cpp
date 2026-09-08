#include <fe/MathUtils.hpp>
#include <fe/Petals.hpp>
#include <fe/detail/FlowerEvolver.hpp>
#include <fe/detail/GenomeValidation.hpp>
#include <fe/3D.hpp>
#include <string>

namespace fe{
namespace detail{

fe::Flower impl_makeFlower(int radius, int numLayers, float P, float bias) noexcept{
	EvoAI::randomGen().setSeed(std::chrono::steady_clock::now().time_since_epoch().count());
	return fe::Flower({0.f, 0.f}, radius, numLayers, P, bias);
}

fe::Flower impl_makePetals(int radius, int numLayers, float P, float bias) noexcept{
	EvoAI::randomGen().setSeed(std::chrono::steady_clock::now().time_since_epoch().count());
	return fe::Flower({0.f, 0.f}, radius, numLayers, P, bias, fe::Petals::Type::Petals);
}

fe::Flower impl_makePetalLayer(int radius, int numLayers, float P, float bias, int layer) noexcept{
	EvoAI::randomGen().setSeed(std::chrono::steady_clock::now().time_since_epoch().count());
	auto petals = [&](){
		auto petals = fe::Petals();
		petals.radius = std::clamp(radius, 4, 256);
		petals.numLayers = std::clamp(numLayers, 1, fe::getTimesDivisibleBy(petals.radius, 2));
		petals.P = P;
		petals.bias = bias;
		petals.image.create(radius*2, radius*2, fe::Color::Transparent);
		return petals;
	}();
	auto dna = fe::DNA();
	dna.add(EvoAI::Genome(4,14,false,true));
	dna.add(EvoAI::Genome(4,4,false,true));
	fe::drawLayer(petals, dna[1], layer);
	fe::Flower result;
	result.dna = std::move(dna);
	result.petals = std::move(petals);
	return result;
}

fe::Flower impl_makeStem(int radius, int numLayers, float P, float bias) noexcept{
	EvoAI::randomGen().setSeed(std::chrono::steady_clock::now().time_since_epoch().count());
	return fe::Flower({0.f, 0.f}, radius, numLayers, P, bias, fe::Petals::Type::Trunk);
}

fe::Image impl_drawFlower(const fe::DNA& dna, int radius, int numLayers, float P, float bias){
	EvoAI::randomGen().setSeed(std::chrono::steady_clock::now().time_since_epoch().count());
	auto paintedFlower = fe::Flower({0.0, 0.0}, radius, numLayers, P, bias, fe::DNA(dna));
	return std::move(paintedFlower.petals.image);
}

fe::Image impl_drawPetals(const fe::DNA& dna, int radius, int numLayers, float P, float bias){
	EvoAI::randomGen().setSeed(std::chrono::steady_clock::now().time_since_epoch().count());
	auto paintedFlower = fe::Flower({0.0, 0.0}, radius, numLayers, P, bias, fe::DNA(dna), fe::Petals::Type::Petals);
	return std::move(paintedFlower.petals.image);
}

fe::Image impl_drawPetalLayer(const fe::DNA& dna, int radius, int numLayers, float P, float bias, int layer){
	EvoAI::randomGen().setSeed(std::chrono::steady_clock::now().time_since_epoch().count());
	if(dna.size() < 2 || !fe::detail::hasUsableOutputs(dna[1], 4)){
		throw std::invalid_argument("invalid DNA, it should have 2 genomes");
	}
	auto petals = [&](){
		auto petals = fe::Petals();
		petals.radius = std::clamp(radius, 4, 256);
		petals.numLayers = std::clamp(numLayers, 1, fe::getTimesDivisibleBy(petals.radius, 2));
		petals.P = P;
		petals.bias = bias;
		petals.image.create(radius*2, radius*2, fe::Color::Transparent);
		return petals;
	}();
	EvoAI::Genome g(dna[1]);
	fe::drawLayer(petals, g, layer);
	return std::move(petals.image);
}

fe::Flower impl_reproduce(const fe::DNA& dna1, const fe::DNA& dna2, int radius, int numLayers, float P, float bias){
	EvoAI::randomGen().setSeed(std::chrono::steady_clock::now().time_since_epoch().count());
	fe::DNA d1(dna1);
	fe::DNA d2(dna2);
	return fe::Flower({0.0, 0.0}, radius, numLayers, P, bias, fe::DNA::reproduce(d1, d2));
}

fe::Flower impl_mutate(fe::DNA original, int radius, int numLayers, float P, float bias, 
					float addNodeRate, float addConnRate, float removeConnRate, float perturbWeightsRate, 
					float enableRate, float disableRate, float actTypeRate){
	EvoAI::randomGen().setSeed(std::chrono::steady_clock::now().time_since_epoch().count());
	original.mutate(fe::MutationRates(addNodeRate, addConnRate, removeConnRate, perturbWeightsRate, enableRate, disableRate, actTypeRate));
	return fe::Flower({0.0, 0.0}, radius, numLayers, P, bias, std::move(original));
}

fe::Stats impl_getFlowerStats(const std::string& genome, float humidity, int temperature, int altitude, int terrainType){
	EvoAI::randomGen().setSeed(std::chrono::steady_clock::now().time_since_epoch().count());
	return fe::Stats{genome, humidity, temperature, altitude, terrainType};
}

std::string impl_make3DFlower(const fe::DNA& dna, int radius, int numLayers, float P, float bias, const std::string& flowerId, const std::string& flowerParams){
	EvoAI::randomGen().setSeed(std::chrono::steady_clock::now().time_since_epoch().count());
	fe::FlowerParameters params = [&](){
		if(!flowerParams.empty()){
			JsonBox::Value v;
			v.loadFromString(flowerParams);
			return fe::FlowerParameters(v.getObject());
		}else{
			return fe::FlowerParameters{};
		}
	}();
	/**
	 * @brief adjust the stem / pistil / stamen radius and height size.
	 */
	auto adjustStem = [](double radius, fe::FlowerParameters& params, double numLayers, bool shortPistilAndStamen){
		constexpr double normMin = 4.0 / 256.0;
		constexpr double normMax = 256.0 / 256.0;
		const double normRadius = radius / 256.0;
		params.stemRadius = EvoAI::normalize(static_cast<double>(normRadius), 0.005, 0.01, normMin, normMax);
		params.pistilStyleRadius = EvoAI::normalize(static_cast<double>(normRadius), 0.00050, 0.001, normMin, normMax);
		params.stamenFilamentRadius = EvoAI::normalize(static_cast<double>(normRadius), 0.00050, 0.001, normMin, normMax);
		auto maxCutPercent = 1.0;
		if(shortPistilAndStamen){
			maxCutPercent = 0.65;
		}
		params.pistilStyleHeight *= EvoAI::normalize(numLayers, 0.4, maxCutPercent, 1.0, 8.0);
		params.stamenFilamentHeight *= EvoAI::normalize(numLayers, 0.4, maxCutPercent, 1.0, 8.0);
	};
	/**
	 * @brief adjust the droop for the petals
	 */
	auto adjustParams = [](int currentLayer, const fe::Petals& p, fe::FlowerParameters& params){
		const float D0 = params.petalDroopFactor;
		constexpr float target = (fe::Math::PI * fe::Math::PI);
		float L_prime = static_cast<float>(p.numLayers - currentLayer) + (D0 * 0.001);
		float newDroop = 0.0f;
		params.petalScaleFactor = EvoAI::normalize(static_cast<double>(p.radius / 256.0), 0.004, 0.008, 4.0 / 256.0, 1.0);
		if(p.bias > 0.0f){
			// For positive bias, we want petals to rise upward.
			// Start at a low magnitude (at outer layer: -D0) and grow to near -target as we move inward.
			constexpr float k_rise = 0.15f;
			newDroop = -(D0 + (target - D0) * (1.0f - std::exp(-k_rise * L_prime)));
		}else if(p.bias < 0.0f){
			// A gentler rate so that the droop is reduced for inner layers
			const float D_min = D0 * 0.5f;
			constexpr float k_droop = 0.075f;
			newDroop = D0 - (D0 - D_min) * (1.0f - std::exp(-k_droop * L_prime));
		}else{
			newDroop = D0;
		}
		params.petalDroopFactor = newDroop;
	};
        if(numLayers <= 0 || radius <= 0 || flowerId.empty()){
            throw std::invalid_argument("Flower3D Error: Invalid input parameters (numLayers=" + std::to_string(numLayers) +
                  ", radius=" + std::to_string(radius) + ", id='" + flowerId + "')");
        }
        if(dna.size() < 2 || !fe::detail::hasUsableOutputs(dna[1], 4)){
            throw std::invalid_argument("invalid DNA, it should have 2 genomes");
        }
	fe::gltf::Scene scene(flowerId);
	fe::gltf::Material stem_material_props = fe::gltf::Material::createStemMaterial();
	int stem_mat_idx = scene.addMaterial(stem_material_props);
	// Start layers slightly above stem
    float current_layer_base_y = params.stemHeight + 0.01f;
	auto currentRadius = std::clamp(radius, 4, 256);
	auto maxNumLayers = std::clamp(numLayers, 1, fe::getTimesDivisibleBy(currentRadius, 2));
	adjustStem(currentRadius, params, static_cast<double>(maxNumLayers), bias <= 0.0);
	fe::generateStem(scene, params, stem_mat_idx);
	fe::gltf::Material pistil_Filament_material_props = fe::gltf::Material::createPistilStyleMaterial();
	int pistil_Filament_mat_idx = scene.addMaterial(pistil_Filament_material_props);
	auto stigma_normal_tex = fe::gltf::TextureInfo("stigma_normal", fe::resources::stigma_normal_texture);
	auto stigma_normal_tex_idx = scene.addTexture(stigma_normal_tex);
	fe::gltf::Material pistil_stigma_material_props = fe::gltf::Material::createPistilStigmaMaterial(stigma_normal_tex_idx);
	int pistil_stigma_mat_idx = scene.addMaterial(pistil_stigma_material_props);
	auto anther_normal_tex = fe::gltf::TextureInfo("anther_normal", fe::resources::anther_normal_texture);
	auto anther_normal_tex_idx = scene.addTexture(anther_normal_tex);
	fe::gltf::Material stamen_filament_material_props = fe::gltf::Material::createStamenFilamentMaterial();
	int stamen_filament_mat_idx = scene.addMaterial(stamen_filament_material_props);
	fe::gltf::Material stamen_anther_material_props = fe::gltf::Material::createStamenAntherMaterial(anther_normal_tex_idx);
	int stamen_anther_mat_idx = scene.addMaterial(stamen_anther_material_props);
	auto center = fe::Vec3f(0.0f, current_layer_base_y, 0.0f);
	float angleStep = (2.0f * fe::Math::PI) / static_cast<float>(params.stamenCount);
	float distance = params.pistilStyleRadius * 2.0f;
	if(params.sex == 0){
		for(auto i=0;i<params.stamenCount;++i){
			float angle = i * angleStep;
			float x = center.x + distance * std::cos(angle);
			float z = center.z + distance * std::sin(angle);
			auto position = fe::Vec3f(x, center.y, z);
			fe::generateStamen(scene, position, params, stamen_filament_mat_idx, stamen_anther_mat_idx, i);
		}
	}else if(params.sex == 1){
		fe::generatePistil(scene, {0.0f, params.stemHeight, 0.0f}, params, pistil_Filament_mat_idx, pistil_stigma_mat_idx, 0);
	}else{
		fe::generatePistil(scene, {0.0f, params.stemHeight, 0.0f}, params, pistil_Filament_mat_idx, pistil_stigma_mat_idx, 0);
		for(auto i=0;i<params.stamenCount;++i){
			float angle = i * angleStep;
			float x = center.x + distance * std::cos(angle);
			float z = center.z + distance * std::sin(angle);
			auto position = fe::Vec3f(x, center.y, z);
			fe::generateStamen(scene, position, params, stamen_filament_mat_idx, stamen_anther_mat_idx, i);
		}
	}
	EvoAI::Genome g(dna[1]);
	for(int layerIdx = maxNumLayers; layerIdx>=0; --layerIdx){
		auto ptls = [&](){
			auto petals = fe::Petals();
			petals.radius = currentRadius;
			petals.numLayers = maxNumLayers;
			petals.P = P;
			petals.bias = bias;
			petals.image.create(currentRadius*2, currentRadius*2, fe::Color::Transparent);
			return petals;
		}();
		currentRadius /= 2.0;
		if(currentRadius < 1){
			continue;
		}
        try{
            fe::drawLayer(ptls, g, layerIdx, false);
            const auto& imageData = ptls.image.imageData;
            int imgWidth = static_cast<int>(ptls.image.mWidth);
            int imgHeight = static_cast<int>(ptls.image.mHeight);
            if(imgWidth <= 0 || imgHeight <= 0 || imageData.empty()){
                continue;
            }
            std::vector<fe::Vec2i> boundaryPoints;
            auto boundaryFound = fe::findContourMoore(imageData, imgWidth, imgHeight, params.alphaThreshold, boundaryPoints);
            if(!boundaryFound || boundaryPoints.size() < 3){
                continue;
            }
            std::vector<fe::Vec2i> simplifiedContour;
            fe::simplifyContour(boundaryPoints, params.contourSimplificationTolerance, simplifiedContour);
            if(simplifiedContour.size() < 3){
                continue;
            }
            adjustParams(layerIdx, ptls, params);
            // 2e. Generate the 3D geometry for this layer
            fe::generatePetalLayer(scene, simplifiedContour, ptls.image, layerIdx, {0.0f, current_layer_base_y, 0.0f}, params);
            // 2f. Update base Y for the next layer (stacking upwards)
            current_layer_base_y += params.layerVerticalSpacing;
        }catch(const std::exception& e){
            // skip problematic layer
            continue;
        }
    }
    std::vector<int> childrenIndices;
    childrenIndices.reserve(scene.nodes.size());
    for(auto i=0u;i<scene.nodes.size();++i){
        bool isGroup = scene.nodes[i].name->find("_Group_") != std::string::npos;
        bool isStem = scene.nodes[i].name->find("Stem") != std::string::npos;
        bool isPetal = scene.nodes[i].name->find("Petal_") != std::string::npos;
        if(isGroup || isStem || isPetal){
            childrenIndices.emplace_back(static_cast<int>(i));
        }
    }
    // add group for Flower_{id}
    scene.addNode(fe::gltf::Node::makeGroup("Flower_" + flowerId, childrenIndices));
    std::string jsonString = "";
     try{
         auto json = fe::gltf::toJson(scene, params);
         jsonString = fe::gltf::toJsonStr(json);
     }catch(const std::exception& e){
         throw std::invalid_argument("make3DFlower() - error Exception during gltf string generation.");
     }
    return jsonString;
}

} // namespace detail
} // namespace fe
