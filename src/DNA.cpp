#include <DNA.hpp>

namespace fe{
	MutationRates::MutationRates()
	: addNodeRate(0.2)
	, addConnRate(0.3)
	, removeConnRate(0.2)
	, perturbWeightsRate(0.6)
	, enableRate(0.35)
	, disableRate(0.3)
	, actTypeRate(0.4){}
	MutationRates::MutationRates(JsonBox::Object o)
	: addNodeRate(o["addNodeRate"].getFloat())
	, addConnRate(o["addConnRate"].getFloat())
	, removeConnRate(o["removeConnRate"].getFloat())
	, perturbWeightsRate(o["perturbWeightsRate"].getFloat())
	, enableRate(o["enableRate"].getFloat())
	, disableRate(o["disableRate"].getFloat())
	, actTypeRate(o["actTypeRate"].getFloat()){}
	MutationRates::MutationRates(float addNodeRate, float addConnRate, float removeConnRate, float perturbWeightsRate, float enableRate, float disableRate, float actTypeRate)
	: addNodeRate(addNodeRate)
	, addConnRate(addConnRate)
	, removeConnRate(removeConnRate)
	, perturbWeightsRate(perturbWeightsRate)
	, enableRate(enableRate)
	, disableRate(disableRate)
	, actTypeRate(actTypeRate){}
	JsonBox::Value MutationRates::toJson() const noexcept{
		JsonBox::Object o;
		o["addNodeRate"] = addNodeRate;
		o["addConnRate"] = addConnRate;
		o["removeConnRate"] = removeConnRate;
		o["perturbWeightsRate"] = perturbWeightsRate;
		o["enableRate"] = enableRate;
		o["disableRate"] = disableRate;
		o["actTypeRate"] = actTypeRate;
		return JsonBox::Value(o);
	}
	DNA::DNA()
	: genomes(){}
	DNA::DNA(DNA&& dna)
	: genomes(std::forward<std::vector<EvoAI::Genome>>(dna.genomes)){}
	DNA::DNA(const DNA& dna)
	: genomes(){
		for(auto& g:dna.genomes){
			add(g);
		}
	}
	DNA::DNA(JsonBox::Object o)
	: genomes(){
		auto& arrGen = o["genomes"].getArray();
		genomes.reserve(arrGen.size());
		for(auto& g:arrGen){
			genomes.emplace_back(g.getObject());
		}
	}
	JsonBox::Value DNA::toJson() const noexcept{
		JsonBox::Object o;
		JsonBox::Array ar;
		ar.reserve(genomes.size());
		for(auto& g:genomes){
			ar.emplace_back(g.toJson());
		}
		o["genomes"] = JsonBox::Value(ar);
		return JsonBox::Value(o);
	}
	DNA::DNA(std::vector<EvoAI::Genome>&& g)
	: genomes(std::forward<std::vector<EvoAI::Genome>>(g)){}
	void DNA::setFitness(double fit) noexcept{
		for(auto& g:genomes){
			g.setFitness(fit);
		}
	}
	double DNA::getFitness() noexcept{
		if(!genomes.empty()){
			return genomes[0].getFitness();
		}
		return 0.0;
	}
	DNA& DNA::add(EvoAI::Genome&& g) noexcept{
		std::size_t id = size();
		genomes.emplace_back(std::forward<EvoAI::Genome>(g));
		genomes.back().setID(id);
		return *this;
	}
	DNA& DNA::add(const EvoAI::Genome& g) noexcept{
		std::size_t id = size();
		genomes.emplace_back(g);
		genomes.back().setID(id);
		return *this;
	}
	bool DNA::remove(EvoAI::Genome* g) noexcept{
		return genomes.erase(std::remove_if(std::begin(genomes),std::end(genomes),
			[&](auto& g2){
				return (*g == g2);
			}),std::end(genomes)) == std::end(genomes);
	}
	void DNA::mutate(const MutationRates& mr) noexcept{
		for(auto& g:genomes){
			g.mutate(mr.addNodeRate,mr.addConnRate,mr.removeConnRate,mr.perturbWeightsRate,mr.enableRate,mr.disableRate,mr.actTypeRate);
		}
	}
	std::size_t DNA::size() const noexcept{
		return genomes.size();
	}
	void DNA::clear() noexcept{
		genomes.clear();
	}
	DNA DNA::reproduce(DNA& dna1, DNA& dna2){
		DNA dna;
		if(dna1.size() != dna2.size()){
			throw std::runtime_error("DNA::reproduce() - dna1 and dna2 size should match.");
		}
		for(auto i=0u;i<dna1.size();++i){
			dna.add(EvoAI::Genome::reproduce(dna1[i], dna2[i]));
		}
		return dna;
	}
	double DNA::distance(DNA& dna1, DNA& dna2){
		double dst = 0.0;
		if(dna1.size() != dna2.size()){
			throw std::runtime_error("DNA::distance() - dna1 and dna2 size should match.");
		}
		for(auto i=0u;i<dna1.size();++i){
			dst += EvoAI::Genome::distance(dna1[i],dna2[i]);
		}
		return dst;
	}
	void DNA::operator=(const DNA& rhs) noexcept{
		clear();
		for(auto& g:rhs.genomes){
			add(g);
		}
	}
	void DNA::operator=(DNA&& rhs) noexcept{
		clear();
		genomes = std::move(rhs.genomes);
		rhs.clear();
	}
	EvoAI::Genome& DNA::operator[](const std::size_t& index) noexcept{
		return genomes[index];
	}
	const EvoAI::Genome& DNA::operator[](const std::size_t& index) const noexcept{
		return genomes[index];
	}
}
