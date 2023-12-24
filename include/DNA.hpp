#ifndef FLOWER_EVOLVER_DNA_HPP
#define FLOWER_EVOLVER_DNA_HPP

#include <vector>
#include <EvoAI.hpp>
#include <JsonBox.h>

namespace fe{
	/**
	 *  @brief Mutation Rates
	 */
	struct MutationRates{
		/**
		 *  @brief default constructor
		 *  default values
		 *  * addNodeRate : 0.2
		 *  * addConnRate : 0.3
		 *  * removeConnRate : 0.2
		 *  * perturbWeightsRate : 0.6
		 *  * enableRate : 0.35
		 *  * disableRate : 0.3
		 *  * actTypeRate : 0.4
		 */
		MutationRates();
		/**
		 *  @brief constructor with all the parameters
		 *  
		 *  @param [in] addNodeRate        rate to add nodes
		 *  @param [in] addConnRate        rate to add connections
		 *  @param [in] removeConnRate     rate to remove connections
		 *  @param [in] perturbWeightsRate rate to change weights of a connection
		 *  @param [in] enableRate         rate to enable a dormant gene
		 *  @param [in] disableRate        rate to disable a enabled gene
		 *  @param [in] actTypeRate        rate to change activation function of a neuronGene
		 */
		MutationRates(float addNodeRate, float addConnRate, float removeConnRate, float perturbWeightsRate, float enableRate, float disableRate, float actTypeRate);
		/**
		 *  @brief load a JsonBox::Object
		 *  
		 *  @param [in] o JsonBox::Object
		 */
		MutationRates(JsonBox::Object o);
		/**
		 *  @brief converts to JsonBox::Value
		 *  @return JsonBox::Value
		 */
		JsonBox::Value toJson() const noexcept;
		float addNodeRate;
		float addConnRate;
		float removeConnRate;
		float perturbWeightsRate;
		float enableRate;
		float disableRate;
		float actTypeRate;
	};
	/**
	 *  @brief DNA Component
	 */
	class DNA final{
		public:
			/**
			 *  @brief default constructor
			 */
			DNA();
			/**
			 *  @brief move constructor
			 */
			DNA(DNA&& dna);
			/**
			 *  @brief copy constructor
			 */
			DNA(const DNA& dna);
			/**
			 *  @brief load a JsonBox::Object containing a DNA
			 *  @param [in] o JsonBox::Object
			 */
			DNA(JsonBox::Object o);
			/**
			 *  @brief converts to JsonBox::Value
			 *  @return JsonBox::Value
			 */
			JsonBox::Value toJson() const noexcept;
			/**
			 *  @brief transfer a std::vector of genomes
			 *  
			 *  @param [in] g std::vector<EvoAI::Genome>&&
			 */
			DNA(std::vector<EvoAI::Genome>&& g);
			/**
			 *  @brief sets the same fitness for all the genomes in DNA
			 *  
			 *  @param [in] fit double fitness
			 */
			void setFitness(double fit) noexcept;
			/**
			 *  @brief gets fitness from genome[0]
			 *  @return double if genomes is empty returns 0.0d
			 */
			double getFitness() noexcept;
			/**
			 *  @brief adds a Genome
			 *  
			 *  @param g EvoAI::Genome
			 *  @return DNA&
			 */
			DNA& add(EvoAI::Genome&& g) noexcept;
			/**
			 *  @brief adds a Genome
			 *  
			 *  @param g const EvoAI::Genome&
			 *  @return DNA&
			 */			
			DNA& add(const EvoAI::Genome& g) noexcept;
			/**
			 *  @brief removes a EvoAI::Genome from DNA.
			 *  
			 *  @param [in] g EvoAI::Genome* genome to remove
			 *  @return bool true if removed
			 */
			bool remove(EvoAI::Genome* g) noexcept;
			/**
			 *  @brief mutates all genomes
			 *  
			 *  @param [in] mr Mutation Rates
			 */
			void mutate(const MutationRates& mr) noexcept;
			/**
			 *  @brief get number of genomes
			 *  
			 *  @return std::size_t number of genomes
			 */
			std::size_t size() const noexcept;
			/**
			 *  @brief clears the DNA
			 */
			void clear() noexcept;
			~DNA() = default;
		public:
			/**
			 *  @brief makes a new DNA from two others(they need to have the same number of genomes).
			 *  
			 *  @param [in] dna1 DNA&
			 *  @param [in] dna2 DNA&
			 *  @throw std::runtime_error if number of genomes are not the same.
			 *  @return DNA
			 */
			static DNA reproduce(DNA& dna1, DNA& dna2);
			/**
			 *  @brief calculates the distance between DNA(they need to have the same number of genomes).
			 *  
			 *  @param [in] dna1 DNA&
			 *  @param [in] dna2 DNA&
			 *  @throw std::runtime_error if number of genomes are not the same.
			 *  @return double distance between two DNA
			 */
			static double distance(DNA& dna1, DNA& dna2);
		public:
			/**
			 *  @brief clears the DNA and copies the other
			 *  @param [in] rhs const DNA&
			 */
			void operator=(const DNA& rhs) noexcept;
			/**
			 *  @brief moves rhs into this.
			 */
			void operator=(DNA&& rhs) noexcept;
			/**
			 *  @brief direct access to genomes
			 *  @param [in] index std::size_t
			 *  @return EvoAI::Genome&
			 */
			EvoAI::Genome& operator[](const std::size_t& index) noexcept;
			/**
			 *  @brief direct access to genomes
			 *  @param [in] index std::size_t
			 *  @return const EvoAI::Genome&
			 */                
			const EvoAI::Genome& operator[](const std::size_t& index) const noexcept;
		private:
			std::vector<EvoAI::Genome> genomes;
	};
}

#endif // FLOWER_EVOLVER_DNA_HPP