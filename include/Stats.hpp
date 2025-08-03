#ifndef FLOWER_EVOLVER_STATS_HPP
#define FLOWER_EVOLVER_STATS_HPP

#include <string>

#include <JsonBox.h>

#include <DNA.hpp>
#include <Petals.hpp>

namespace fe{
	/**
	 * @brief storage class for stats
	 */
	struct Stats final{
		/**
		 * @brief enum for Sex
		 */
		enum Sex{
			Male = 0,
			Female = 1,
			Both = 2
		};
		/**
		 * @brief storage class for Effects
		 */
		struct Effects{
			Effects();
			// data
			double vitality;
			double agility;
			double intelligence;
			double strength;
			double luck;
		};
		/**
		 * @brief constructor to build stats from the flower genome.
		 * @param genome const std::string& stringified flower.json
		 * @param humidity float 0.0 to 1.0
		 * @param temperature int temperature
		 * @param altitude int meters above sea
		 * @param terrainType int terrain type
		 */
		Stats(const std::string& genome, float humidity, int temperature, int altitude, int terrainType);
		/**
		 * @brief convert to json
		 * @return JsonBox::Object
		 */
		JsonBox::Object toJson() const noexcept;
		// data
		Sex sex;
		int health;
		int stamina;
		int minTemperature;
		int maxTemperature;
		int maturationPeriod;
		double toxicityRate;
		Effects effects;
	};
} // namespace fe

#endif // FLOWER_EVOLVER_STATS_HPP