#include <fe/FlowerClassifier.hpp>

#include <fe/detail/FlowerEvolver.hpp>
#include <fe/detail/GenomeValidation.hpp>

#include <EvoAI/Utils/MathUtils.hpp>

#include <algorithm>
#include <cmath>
#include <iostream>
#include <limits>
#include <sstream>
#include <stdexcept>

namespace{
	// compact, like every other JSON producer here
	std::string jsonToString(const JsonBox::Value& v){
		std::stringstream ss;
		v.writeToStream(ss, false, true);
		return ss.str();
	}

	// MAX-pooled, not averaged: at extreme P petals can be one pixel wide and a mean
	// over an 8x8 block would nearly erase them.
	std::vector<double> downsamplePetalsSilhouette(const fe::Image& img, std::size_t outW, std::size_t outH) noexcept{
		auto size = img.getSize();
		auto srcW = static_cast<std::size_t>(size.x);
		auto srcH = static_cast<std::size_t>(size.y);
		std::vector<double> out(outW * outH, 0.0);
		if(srcW == 0 || srcH == 0){
			return out;
		}
		for(auto oy = 0u; oy < outH; ++oy){
			auto y0 = (oy * srcH) / outH;
			auto y1 = std::max(y0 + 1, ((oy + 1) * srcH) / outH);
			for(auto ox = 0u; ox < outW; ++ox){
				auto x0 = (ox * srcW) / outW;
				auto x1 = std::max(x0 + 1, ((ox + 1) * srcW) / outW);
				fe::Uint8 maxAlpha = 0;
				for(auto y = y0; y < y1 && y < srcH; ++y){
					for(auto x = x0; x < x1 && x < srcW; ++x){
						maxAlpha = std::max(maxAlpha, img.getPixel(x, y).a);
					}
				}
				out[oy * outW + ox] = maxAlpha / 255.0;
			}
		}
		return out;
	}

	// Stats fields are hard-clamped at the source (see Stats.cpp), so a plain (v-min)/(max-min)
	// with fixed bounds is safe and needs no fitted scaler.
	// Order must match fe::FlowerClassifier::statNames().
	std::vector<double> statsToFeatureVector(const fe::Stats& s) noexcept{
		return {
			EvoAI::stableNormalize<double>(s.health, 1, 100),
			EvoAI::stableNormalize<double>(s.stamina, 1, 100),
			EvoAI::stableNormalize<double>(s.minTemperature, 0, 50),
			EvoAI::stableNormalize<double>(s.maxTemperature, 0, 50),
			EvoAI::stableNormalize<double>(s.maturationPeriod, 1, 100),
			EvoAI::stableNormalize<double>(s.toxicityRate, -100, 100),
			EvoAI::stableNormalize<double>(s.effects.vitality, -100, 100),
			EvoAI::stableNormalize<double>(s.effects.agility, -100, 100),
			EvoAI::stableNormalize<double>(s.effects.intelligence, -100, 100),
			EvoAI::stableNormalize<double>(s.effects.strength, -100, 100),
			EvoAI::stableNormalize<double>(s.effects.luck, -100, 100),
		};
	}
}

namespace fe{
    std::vector<double> extractFlowerFeatures(const std::string& genome, const fe::Environment& environment){
        environment.validate();
        JsonBox::Value v;
        v.loadFromString(genome);
        auto flowerObj = v["Flower"].getObject();
        fe::DNA dna(flowerObj["dna"].getObject());
        if(dna.size() < 2 || !fe::detail::hasUsableOutputs(dna[0], 14) || !fe::detail::hasUsableOutputs(dna[1], 4)){
            throw std::invalid_argument("invalid genome, its DNA doesn't have enough usable outputs to extract features.");
        }
        // transient, only to read radius/numLayers/P/bias (the constructor does not render).
        // A missing or malformed "petals" section is not an error: Petals falls back to its defaults.
        fe::Petals petals(flowerObj["petals"].getObject());
        auto img = fe::detail::impl_drawPetals(dna, petals.radius, petals.numLayers, petals.P, petals.bias);
        auto features = downsamplePetalsSilhouette(img, FlowerClassifier::silhouetteSize, FlowerClassifier::silhouetteSize);

        // fe::Stats is string-based, so hand it the original `genome` as is
        auto stats = fe::detail::impl_getFlowerStats(genome,
            environment.humidity, environment.temperature, environment.altitude, environment.terrainType);
        auto statsVec = statsToFeatureVector(stats);

        features.insert(features.end(), statsVec.begin(), statsVec.end());
        return features;
    }

    const std::vector<std::string>& FlowerClassifier::statNames() noexcept{
        static const std::vector<std::string> names{
            "health", "stamina", "minTemperature", "maxTemperature", "maturationPeriod",
            "toxicityRate", "vitality", "agility", "intelligence", "strength", "luck"
        };
        return names;
    }

    FlowerClassifier::FlowerClassifier(std::size_t width, std::size_t height, fe::Environment environment)
    : som(width, height, inputDim)
    , environment(environment){}
    FlowerClassifier::FlowerClassifier(JsonBox::Object o)
    : som(o["som"].getObject())
    , environment(o["environment"].getObject()){
        if(som.getInputDim() != inputDim){
            throw std::invalid_argument("fe::FlowerClassifier - loaded classifier has input dimension "
                + std::to_string(som.getInputDim()) + ", this build expects " + std::to_string(inputDim));
        }
    }
    std::optional<SpeciesMatch> FlowerClassifier::classifyFeatures(const std::vector<double>& features) const{
        auto match = som.classify(features);
        if(!match){
            return std::nullopt;
        }
        return SpeciesMatch{match->x, match->y, match->distance};
    }
    std::optional<SpeciesMatch> FlowerClassifier::classify(const std::string& genome, const fe::Environment& environment) const{
        // outside the try: a bad environment is the caller's bug, not a flower that can't be classified
        environment.validate();
        try{
            return classifyFeatures(fe::extractFlowerFeatures(genome, environment));
        }catch(const std::exception&){ // malformed JSON (JsonBox throws std::runtime_error) or unusable DNA (std::invalid_argument)
            return std::nullopt;
        }
    }
    std::optional<SpeciesMatch> FlowerClassifier::classify(const fe::Flower& flower, const fe::Environment& environment) const{
        // field serialization only, not a render
        JsonBox::Object wrapper;
        wrapper["Flower"] = flower.toJson();
        return classify(jsonToString(JsonBox::Value(wrapper)), environment);
    }
    SOMEvaluation FlowerClassifier::evaluate(const std::vector<std::vector<double>>& samples) const{
        return evaluateSOM(som, samples);
    }
    SOMTrainingReport FlowerClassifier::train(const std::vector<std::vector<double>>& samples, std::size_t epochs, double baseLR, double baseRadius,
            std::size_t epochOffset, std::size_t totalEpochs, const SOMEpochCallback& onEpoch){
        // !(x > 0) rather than x <= 0 so NaN is rejected too; with either
        // one the gamma below is inf/NaN and silently ruins every weight.
        if(!(baseLR > 0.0) || !std::isfinite(baseLR)){
            throw std::invalid_argument("fe::FlowerClassifier::train - baseLR must be finite and > 0");
        }
        if(!(baseRadius > 0.0) || !std::isfinite(baseRadius)){
            throw std::invalid_argument("fe::FlowerClassifier::train - baseRadius must be finite and > 0");
        }
        if(epochs > std::numeric_limits<std::size_t>::max() - epochOffset){
            throw std::invalid_argument("fe::FlowerClassifier::train - epochOffset + epochs overflows");
        }
        const std::size_t total = totalEpochs == 0 ? epochOffset + epochs : totalEpochs;
        if(epochOffset + epochs > total){
            throw std::invalid_argument("fe::FlowerClassifier::train - epochOffset + epochs exceeds totalEpochs");
        }
        double total_epochs = static_cast<double>(total == 0 ? 1.0 : total);
        double radius_gamma = std::pow(0.5 / baseRadius, 1.0 / total_epochs);
        double lr_gamma = std::pow(0.01 / baseLR, 1.0 / total_epochs);
        // the SOM replays the schedulers up to epochOffset, so a later call continues
        // the same decay where the earlier calls stopped
        EvoAI::TrainingOptions options;
        options.epochOffset = epochOffset;
        if(onEpoch){
            options.onEpoch = [&onEpoch](const EvoAI::EpochInfo& info){
                return onEpoch(SOMEpoch{info.epoch, info.learningRate, info.radius, info.meanBmuDistance});
            };
        }
        auto report = som.train(samples, epochs, baseLR, baseRadius,
            EvoAI::Scheduler<EvoAI::ExponentialLR>(EvoAI::ExponentialLR{lr_gamma}),
            EvoAI::Scheduler<EvoAI::ExponentialLR>(EvoAI::ExponentialLR{radius_gamma}), options);
        // the SOM returns early without touching its weights here, so the
        // caller would otherwise get back a classifier that looks trained
        if(report.skipped.size() == samples.size()){
            throw std::invalid_argument("fe::FlowerClassifier::train - no usable samples to train on");
        }
        SOMTrainingReport out;
        out.epochs.reserve(report.epochs.size());
        for(const auto& info : report.epochs){
            out.epochs.push_back(SOMEpoch{info.epoch, info.learningRate, info.radius, info.meanBmuDistance});
        }
        out.skipped = std::move(report.skipped);
        out.nodeCounts = std::move(report.nodeCounts);
        return out;
    }
    JsonBox::Value FlowerClassifier::toJson() const noexcept{
        JsonBox::Object o;
        o["version"] = 1;
        JsonBox::Object input;
        input["imageWidth"] = static_cast<int>(silhouetteSize);
        input["imageHeight"] = static_cast<int>(silhouetteSize);
        input["channels"] = 1;
        input["mode"] = std::string("alpha");
        input["normalization"] = std::string("255");
        o["input"] = JsonBox::Value(input);
        JsonBox::Array names;
        for(auto& name : statNames()){
            names.emplace_back(name);
        }
        o["stats"] = JsonBox::Value(names);
        o["environment"] = environment.toJson();
        o["som"] = som.toJson();
        return JsonBox::Value(o);
    }
} // namespace fe
