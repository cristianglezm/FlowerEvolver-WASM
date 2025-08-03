#include <EvoAI/Utils/MathUtils.hpp>
#include <Flower.hpp>

namespace fe{
	Flower::Flower()
	: dna()
	, petals(){}
	Flower::Flower([[maybe_unused]] const sf::Vector2f& pos, int radius, int numLayers, float P, float bias)
	: dna()
	, petals(radius, numLayers, P, bias){
		//add stats genome so is compatible with EcoSystem Flowers
		dna.add(EvoAI::Genome(4,14,false,true));
		dna.add(EvoAI::Genome(4,4,false,true));
		draw(Petals::Type::TrunkAndPetals, petals, dna[1]);
	}
	Flower::Flower([[maybe_unused]] const sf::Vector2f& pos, int radius, int numLayers, float P, float bias, const Petals::Type& type)
	: dna()
	, petals(radius, numLayers, P, bias){
		//add stats genome so is compatible with EcoSystem Flowers
		dna.add(EvoAI::Genome(4,14,false,true));
		dna.add(EvoAI::Genome(4,4,false,true));
		draw(type, petals, dna[1]);
	}
	Flower::Flower([[maybe_unused]] const sf::Vector2f& pos, int radius, int numLayers, float P, float bias, DNA&& dna, const Petals::Type& type)
	: dna(std::forward<DNA>(dna))
	, petals(radius, numLayers, P, bias){
		if(this->dna.size() < 2){
			throw std::invalid_argument("invalid DNA, it should have 2 genomes");
		}
		draw(type, petals, this->dna[1]);
	}
	Flower::Flower([[maybe_unused]] const sf::Vector2f& pos, int radius, int numLayers, float P, float bias, DNA&& dna)
	: dna(std::forward<DNA>(dna))
	, petals(radius, numLayers, P, bias){
		if(this->dna.size() < 2){
			throw std::invalid_argument("invalid DNA, it should have 2 genomes");
		}
		draw(Petals::Type::TrunkAndPetals, petals, this->dna[1]);
	}
	Flower::Flower(const Flower& rhs) noexcept
	: dna()
	, petals(rhs.petals){
		for(auto i=0u;i<rhs.dna.size();++i){
			dna.add(rhs.dna[i]);
		}
	}
	Flower::Flower(Flower&& rhs) noexcept
	: dna(std::forward<DNA>(rhs.dna))
	, petals(std::forward<Petals>(rhs.petals)){}
	Flower::Flower(JsonBox::Object o)
	: dna(o["dna"].getObject())
	, petals(o["petals"].getObject()){
		draw(Petals::Type::TrunkAndPetals, petals, dna[1]);
	}
	JsonBox::Value Flower::toJson() const noexcept{
		JsonBox::Object o;
		o["dna"] = dna.toJson();
		o["petals"] = petals.toJson();
		return JsonBox::Value(o);
	}
	void Flower::operator=(Flower&& rhs) noexcept{
		dna = std::move(rhs.dna);
		petals = std::move(rhs.petals);
	}
	void Flower::operator=(const Flower& rhs) noexcept{
		dna.clear();
		for(auto i=0u;i<rhs.dna.size();++i){
			dna.add(rhs.dna[i]);
		}
		petals = rhs.petals;
	}
} // namespace fe
