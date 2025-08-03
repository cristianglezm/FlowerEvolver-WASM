#include <Stats.hpp>
#include <EvoAI/Utils/MathUtils.hpp>

namespace fe{
	Stats::Effects::Effects()
	: vitality{0.0}
	, agility{0.0}
	, intelligence{0.0}
	, strength{0.0}
	, luck{0.0}{}
	Stats::Stats(const std::string& genome, float humidity, int temperature, int altitude, int terrainType)
	: sex{Sex::Both}
	, health{100}
	, stamina{100}
	, minTemperature{0}
	, maxTemperature{100}
	, toxicityRate{0.0}
	, effects{}{
		JsonBox::Value v1;
		v1.loadFromString(genome);
		if(v1["Flower"]["dna"].isNull()){
			throw std::invalid_argument("error, invalid flower, could not parse data.");
		}
		fe::DNA dna(v1["Flower"]["dna"].getObject());
		if(dna.size() < 2){
			throw std::invalid_argument("invalid DNA, it should have 2 genomes");
		}
		auto calcEffect = [](const std::vector<double>& output){
			double result = 0.0;
			if(output.size() < 14){
				return result;
			}
			// it uses outputs[0-5] to calculate effects strength for (vitality, agility, intelligence, strength, luck)
			double sign = output[5] > 0.5 ? 1.0 : -1.0;
			double left = (std::abs(output[0]) + std::abs(output[1]));
			double right = (std::abs(output[3]) + std::abs(output[4]));
			double raw = left * output[2] * right;
			// a controls the linear response (small gaps).
			const double a = 1.5 * std::abs(output[0]);
			// b controls when large differences develop (big gaps).
			const double b = 0.05 * std::abs(output[3]);

			result = std::round(sign * (a * raw + b * std::pow(raw, 3)));
			return result;
		};
		// Genome[0 = stats]
		//   4 inputs [humidity, temperature, altitude, terrain Type | effect id]
		//   14 outputs [5 effects + 1 negative or positive selector]
		//   health, stamina, min temperature, max temperature, maturationPeriod, male, female, toxicityRate
		//   maturationPeriod
		//    how long it takes to bloom, it can be seconds, hours, days, weeks, etc.
		auto nn = EvoAI::Genome::makePhenotype(dna[0]);
		auto normalizeValue = [](auto value){
			return EvoAI::normalize(std::clamp(static_cast<double>(value), -10000.0, 10000.0), -1.0, 1.0, -10000.0, 10000.0);
		};
		double humidityNorm = normalizeValue(humidity);
		double temperatureNorm = normalizeValue(temperature);
		double altitudeNorm = normalizeValue(altitude);
		double terrainTypeNorm = normalizeValue(terrainType);
		// get initial values.
		auto out = nn.forward({humidityNorm, temperatureNorm, altitudeNorm, terrainTypeNorm});
		nn.reset();
		auto getSex = [](double male, double female){
			if(male > 0.5 && female > 0.5){
				return Stats::Sex::Both;
			}else if(female > 0.5){
				return Stats::Sex::Female;
			}else if(male > 0.5){
				return Stats::Sex::Male;
			}
			return Stats::Sex::Both;
		};
		health = std::clamp(static_cast<int>(std::abs(out[6]) * 10), 1, 100);
		stamina = std::clamp(static_cast<int>(std::abs(out[7]) * 10), 1, 100);
		minTemperature = std::clamp(static_cast<int>(std::abs(out[8])) * 10, 0, 50);
		maxTemperature = std::clamp(static_cast<int>(std::abs(out[9])) * 10, 2, 50);
		if(minTemperature > maxTemperature){
			std::swap(minTemperature, maxTemperature);
		}
		maturationPeriod = std::clamp(static_cast<int>(std::abs(out[10])) * 10, 1, 100);
		sex = getSex(out[11], out[12]);
		toxicityRate = std::clamp(std::round(out[13]), -100.0, 100.0);
		using TransformFn = std::function<double(double)>;
		std::array<TransformFn, 5> transforms{
			[](double x){ return std::sin(x); },  // vitality
			[](double x){ return std::cosh(x); }, // agility
			[](double x){ return std::sinh(x); }, // intelligence
			[](double x){ return std::tan(x); },  // strength
			[](double x){ return std::cos(x); }   // luck
		};
		std::size_t effectID = 1000;
		std::array<double Effects::*, 5> effectMembers = {
			&Effects::vitality,
			&Effects::agility,
			&Effects::intelligence,
			&Effects::strength,
			&Effects::luck
		};
		for(auto i=0u;i<5;++i){
			out = nn.forward({
				static_cast<double>(humidityNorm), 
				static_cast<double>(temperatureNorm), 
				static_cast<double>(altitudeNorm), 
				transforms[i](normalizeValue(effectID * i))
			});
			nn.reset();
			effects.*effectMembers[i] = std::clamp(calcEffect(out), -100.0, 100.0);
		}
	}
	JsonBox::Object Stats::toJson() const noexcept{
		JsonBox::Object o;
		o["health"] = health;
		o["stamina"] = stamina;
		o["minTemperature"] = minTemperature;
		o["maxTemperature"] = maxTemperature;
		o["maturationPeriod"] = maturationPeriod;
		if(sex == fe::Stats::Sex::Male){
			o["sex"] = "male";
		}else if(sex == fe::Stats::Sex::Female){
			o["sex"] = "female";
		}else{
			o["sex"] = "both";
		}
		o["toxicityRate"] = toxicityRate;
		JsonBox::Object effectsJson;
		effectsJson["vitality"] = effects.vitality;
		effectsJson["agility"] = effects.agility;
		effectsJson["intelligence"] = effects.intelligence;
		effectsJson["strength"] = effects.strength;
		effectsJson["luck"] = effects.luck;
		o["effects"] = effectsJson;
		return o;
	}
} // namespace fe
