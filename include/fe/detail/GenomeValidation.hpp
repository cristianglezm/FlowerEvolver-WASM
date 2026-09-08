#ifndef FLOWER_EVOLVER_DETAIL_GENOME_VALIDATION_HPP
#define FLOWER_EVOLVER_DETAIL_GENOME_VALIDATION_HPP

#include <EvoAI.hpp>

namespace fe::detail{
	/**
	 * @brief whether g actually builds into a phenotype with enough output
	 * neurons for a forward() call to safely index into.
     *
	 * @param g the genome to check.
	 * @param minOutputs the highest output index the caller is about to
	 * read from forward()'s result, plus one (e.g. 4 for indices 0-3).
	 */
	inline bool hasUsableOutputs(const EvoAI::Genome& g, std::size_t minOutputs) noexcept{
		auto nn = EvoAI::Genome::makePhenotype(g);
		auto& layers = nn.getLayers();
		return !layers.empty() && layers.front().size() > 0 && layers.back().size() >= minOutputs;
	}
}

#endif // FLOWER_EVOLVER_DETAIL_GENOME_VALIDATION_HPP
