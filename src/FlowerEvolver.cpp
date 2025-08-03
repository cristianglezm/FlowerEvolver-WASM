#include <MathUtils.hpp>
#include <Petals.hpp>
#include <FlowerEvolver.hpp>
#include <3D.hpp>
#include <string>

void copyToCanvas(std::uint8_t* ptr, int w, int h){
	EM_ASM_({
                let data = Module.HEAPU8.slice($0, $0 + $1 * $2 * 4);
                if(typeof importScripts === 'function'){
                    let context = self.canvas.getContext('2d', { willReadFrequently: true });
                    let imageData = context.getImageData(0, 0, $1, $2);
                    imageData.data.set(data);
                    context.putImageData(imageData, 0, 0);
                }else{
                    let context = document.getElementById("canvas").getContext('2d', { willReadFrequently: true });
                    let imageData = context.getImageData(0, 0, $1, $2);
                    imageData.data.set(data);
                    context.putImageData(imageData, 0, 0);
                }
    }, ptr, w, h);
}

std::string makeFlower(int radius, int numLayers, float P, float bias) noexcept{
	EvoAI::randomGen().setSeed(std::chrono::steady_clock::now().time_since_epoch().count());
	fe::Flower flower({0.f, 0.f}, radius, numLayers, P, bias);
	auto size = flower.petals.image.getSize();
	copyToCanvas(flower.petals.image.imageData.data(), size.x, size.y);
	std::stringstream ss;
	JsonBox::Value v;
	v["Flower"] = flower.toJson();
	v.writeToStream(ss, false, true);
	return ss.str();
}

std::string makePetals(int radius, int numLayers, float P, float bias) noexcept{
	EvoAI::randomGen().setSeed(std::chrono::steady_clock::now().time_since_epoch().count());
	fe::Flower flower({0.f, 0.f}, radius, numLayers, P, bias, fe::Petals::Type::Petals);
	auto size = flower.petals.image.getSize();
	copyToCanvas(flower.petals.image.imageData.data(), size.x, size.y);
	std::stringstream ss;
	JsonBox::Value v;
	v["Flower"] = flower.toJson();
	v.writeToStream(ss, false, true);
	return ss.str();
}

std::string makePetalLayer(int radius, int numLayers, float P, float bias, int layer) noexcept{
	EvoAI::randomGen().setSeed(std::chrono::steady_clock::now().time_since_epoch().count());
	auto petals = [&](){
		auto petals = fe::Petals();
		petals.radius = std::clamp(radius, 4, 256);
		petals.numLayers = std::clamp(numLayers, 1, fe::getTimesDivisibleBy(petals.radius, 2));
		petals.P = P;
		petals.bias = bias;
		petals.image.create(radius*2, radius*2, sf::Color::Transparent);
		return petals;
	}();
	auto dna = fe::DNA();
	dna.add(EvoAI::Genome(4,14,false,true));
	dna.add(EvoAI::Genome(4,4,false,true));
	fe::drawLayer(petals, dna[1], layer);
	auto size = petals.image.getSize();
	copyToCanvas(petals.image.imageData.data(), size.x, size.y);
	std::stringstream ss;
	JsonBox::Object o;
	o["dna"] = dna.toJson();
	o["petals"] = petals.toJson();
	JsonBox::Value v;
	v["Flower"] = JsonBox::Value(o);
	v.writeToStream(ss, false, true);
	return ss.str();
}

std::string makeStem(int radius, int numLayers, float P, float bias) noexcept{
	EvoAI::randomGen().setSeed(std::chrono::steady_clock::now().time_since_epoch().count());
	fe::Flower flower({0.f, 0.f}, radius, numLayers, P, bias, fe::Petals::Type::Trunk);
	auto size = flower.petals.image.getSize();
	copyToCanvas(flower.petals.image.imageData.data(), size.x, size.y);
	std::stringstream ss;
	JsonBox::Value v;
	v["Flower"] = flower.toJson();
	v.writeToStream(ss, false, true);
	return ss.str();
}

void drawFlower(const std::string& flower, int radius, int numLayers, float P, float bias){
	EvoAI::randomGen().setSeed(std::chrono::steady_clock::now().time_since_epoch().count());
	JsonBox::Value v1;
	v1.loadFromString(flower);
	if(v1["Flower"]["dna"].isNull()){
		throw std::invalid_argument("error, invalid flower, could not parse data.");
	}
	fe::DNA dna(v1["Flower"]["dna"].getObject());
	auto paintedFlower = fe::Flower({0.0, 0.0}, radius, numLayers, P, bias, std::move(dna));
	auto size = paintedFlower.petals.image.getSize();
	copyToCanvas(paintedFlower.petals.image.imageData.data(), size.x, size.y);
}

void drawPetals(const std::string& flower, int radius, int numLayers, float P, float bias){
	EvoAI::randomGen().setSeed(std::chrono::steady_clock::now().time_since_epoch().count());
	JsonBox::Value v1;
	v1.loadFromString(flower);
	if(v1["Flower"]["dna"].isNull()){
		throw std::invalid_argument("error, invalid flower, could not parse data.");
	}
	fe::DNA dna(v1["Flower"]["dna"].getObject());
	auto paintedFlower = fe::Flower({0.0, 0.0}, radius, numLayers, P, bias, std::move(dna), fe::Petals::Type::Petals);
	auto size = paintedFlower.petals.image.getSize();
	copyToCanvas(paintedFlower.petals.image.imageData.data(), size.x, size.y);
}

void drawPetalLayer(const std::string& flower, int radius, int numLayers, float P, float bias, int layer){
	EvoAI::randomGen().setSeed(std::chrono::steady_clock::now().time_since_epoch().count());
	auto petals = [&](){
		auto petals = fe::Petals();
		petals.radius = std::clamp(radius, 4, 256);
		petals.numLayers = std::clamp(numLayers, 1, fe::getTimesDivisibleBy(petals.radius, 2));
		petals.P = P;
		petals.bias = bias;
		petals.image.create(radius*2, radius*2, sf::Color::Transparent);
		return petals;
	}();
	JsonBox::Value v1;
	v1.loadFromString(flower);
	if(v1["Flower"]["dna"].isNull()){
		throw std::invalid_argument("error, invalid flower, could not parse data.");
	}
	fe::DNA dna(v1["Flower"]["dna"].getObject());
	if(dna.size() < 2){
		throw std::invalid_argument("invalid DNA, it should have 2 genomes");
	}
	fe::drawLayer(petals, dna[1], layer);
	auto size = petals.image.getSize();
	copyToCanvas(petals.image.imageData.data(), size.x, size.y);
}

std::string reproduce(const std::string& flower1, const std::string& flower2, int radius, int numLayers, float P, float bias){
	EvoAI::randomGen().setSeed(std::chrono::steady_clock::now().time_since_epoch().count());
	JsonBox::Value v1;
	v1.loadFromString(flower1);
	if(v1["Flower"]["dna"].isNull()){
		throw std::invalid_argument("error, invalid flower1, could not parse data.");
	}
	JsonBox::Value v2;
	v2.loadFromString(flower2);
	if(v2["Flower"]["dna"].isNull()){
		throw std::invalid_argument("error, invalid flower2, could not parse data.");
	}
	fe::DNA dna1(fe::DNA(v1["Flower"]["dna"].getObject()));
	fe::DNA dna2(fe::DNA(v2["Flower"]["dna"].getObject()));
	auto child = fe::Flower({0.0, 0.0}, radius, numLayers, P, bias, fe::DNA::reproduce(dna1, dna2));
	auto size = child.petals.image.getSize();
	copyToCanvas(child.petals.image.imageData.data(), size.x, size.y);
	std::stringstream ss;
	JsonBox::Value v;
	v["Flower"] = child.toJson();
	v.writeToStream(ss, false, true);
	return ss.str();
}

std::string mutate(const std::string& original, int radius, int numLayers, float P, float bias, 
					float addNodeRate, float addConnRate, float removeConnRate, float perturbWeightsRate, 
					float enableRate, float disableRate, float actTypeRate){
	EvoAI::randomGen().setSeed(std::chrono::steady_clock::now().time_since_epoch().count());
	JsonBox::Value v1;
	v1.loadFromString(original);
	if(v1["Flower"]["dna"].isNull()){
		throw std::invalid_argument("error, invalid original, could not parse data.");
	}
	fe::DNA dna(fe::DNA(v1["Flower"]["dna"].getObject()));
	dna.mutate(fe::MutationRates(addNodeRate, addConnRate, removeConnRate, perturbWeightsRate, enableRate, disableRate, actTypeRate));
	auto mutated = fe::Flower({0.0, 0.0}, radius, numLayers, P, bias, std::move(dna));
	auto size = mutated.petals.image.getSize();
	copyToCanvas(mutated.petals.image.imageData.data(), size.x, size.y);
	std::stringstream ss;
	JsonBox::Value v;
	v["Flower"] = mutated.toJson();
	v.writeToStream(ss, false, true);
	return ss.str();
}
std::string getFlowerStats(const std::string& genome, float humidity, int temperature, int altitude, int terrainType){
	EvoAI::randomGen().setSeed(std::chrono::steady_clock::now().time_since_epoch().count());
	fe::Stats stats{genome, humidity, temperature, altitude, terrainType};
	auto o = stats.toJson();
	std::stringstream ss;
	JsonBox::Value v;
	v["stats"] = o;
	v.writeToStream(ss, false, true);
	return ss.str();
}

std::string make3DFlower(const std::string& genome, int radius, int numLayers, float P, float bias, const std::string& flowerId, const std::string& flowerParams){
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
	JsonBox::Value v1;
	v1.loadFromString(genome);
	if(v1["Flower"]["dna"].isNull()){
	    throw std::invalid_argument("error, invalid flower genome, could not parse data.");
	}
	fe::DNA dna(fe::DNA(v1["Flower"]["dna"].getObject()));
        if(dna.size() < 2){
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
	for(int layerIdx = maxNumLayers; layerIdx>=0; --layerIdx){
		auto ptls = [&](){
			auto petals = fe::Petals();
			petals.radius = currentRadius;
			petals.numLayers = maxNumLayers;
			petals.P = P;
			petals.bias = bias;
			petals.image.create(currentRadius*2, currentRadius*2, sf::Color::Transparent);
			return petals;
		}();
		currentRadius /= 2.0;
		if(currentRadius < 1){
			continue;
		}
        try{
            fe::drawLayer(ptls, dna[1], layerIdx, false);
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

std::string getExceptionMessage(int exceptionPtr){
    return std::string(reinterpret_cast<std::exception *>(exceptionPtr)->what());
}
