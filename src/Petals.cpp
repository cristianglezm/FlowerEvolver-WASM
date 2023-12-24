#include <Petals.hpp>

namespace fe{
	Petals::Petals() noexcept
	: image()
	, bias(1.0)
	, P(6.0)
	, radius(64)
	, numLayers(3)
	, hasBloom(false){}
	Petals::Petals(int r, int nLayers, float P, float bias)
	: image()
	, bias(bias)
	, P(P)
	, radius(r)
	, numLayers(nLayers)
	, hasBloom(false){
		image.create(radius*2,radius*3,sf::Color::Transparent);
	}
	Petals::Petals(JsonBox::Object o)
	: image()
	, bias(o["bias"].getFloat())
	, P(o["P"].getFloat())
	, radius(o["radius"].getInteger())
	, numLayers(o["numLayers"].getInteger())
	, hasBloom(false){
		image.create(radius*2,radius*3,sf::Color::Transparent);
	}
	Petals::Petals(const Petals& rhs) noexcept
	: image(rhs.image)
	, bias(rhs.bias)
	, P(rhs.P)
	, radius(rhs.radius)
	, numLayers(rhs.numLayers)
	, hasBloom(false){}
	Petals::Petals(Petals&& rhs) noexcept
	: image(std::move(rhs.image))
	, bias(rhs.bias)
	, P(rhs.P)
	, radius(rhs.radius)
	, numLayers(rhs.numLayers)
	, hasBloom(rhs.hasBloom){}
	JsonBox::Value Petals::toJson() const noexcept{
		JsonBox::Object o;
		o["bias"] = JsonBox::Value(bias);
		o["P"] = JsonBox::Value(P);
		o["radius"] = JsonBox::Value(radius);
		o["numLayers"] = JsonBox::Value(numLayers);
		return JsonBox::Value(o);
	}
	void Petals::operator=(const Petals& rhs) noexcept{
		image = rhs.image;
		bias = rhs.bias;
		P = rhs.P;
		radius = rhs.radius;
		numLayers = rhs.numLayers;
		hasBloom = rhs.hasBloom;
	}
	void Petals::operator=(Petals&& rhs) noexcept{
		image = std::move(rhs.image);
		bias = rhs.bias;
		P = rhs.P;
		radius = rhs.radius;
		numLayers = rhs.numLayers;
		hasBloom = rhs.hasBloom;
	}
	namespace priv{
		std::vector<double> queryNN(EvoAI::NeuralNetwork& nn, Petals& petals, const sf::Vector2f& pos, int currentRadius, int currentLayer) noexcept{
			const auto& origin = sf::Vector2f(petals.radius, petals.radius);
			float angle = std::sin(petals.P*Math::directedAngle(pos,origin, origin));
			auto res = nn.forward({static_cast<float>(currentRadius), angle, static_cast<float>(currentLayer), petals.bias});
			nn.reset();
			return res;
		}
		void setColorAndCut(const sf::Vector2f& pos, EvoAI::NeuralNetwork& nn, Petals& petals, int currentRadius, int currentLayer) noexcept{
			const auto size = petals.image.getSize();
			const auto& bounds = sf::FloatRect(0,0,size.x,size.y);
			const auto& origin = sf::Vector2f(petals.radius,petals.radius);
			const float angle = std::sin(petals.P*Math::directedAngle(pos, origin, origin));
			auto direction = Math::normalize(pos - origin);
			auto res = nn.forward({0.0f, angle, static_cast<float>(currentLayer), petals.bias});
			nn.reset();
			float rMax = std::min(std::abs(res[3] * currentRadius),static_cast<double>(currentRadius));
			auto newPos = origin + direction;
			auto halfLayerSize = petals.numLayers/2;
			for(auto r=0;r<=rMax;++r){
				if(currentLayer >= halfLayerSize){
					res = queryNN(nn,petals,newPos,r,currentLayer);
					currentRadius = res[3];
				}
				if(bounds.contains(newPos)){
					res = queryNN(nn,petals,newPos,currentRadius,currentLayer);
					petals.image.setPixel(newPos.x,newPos.y, sf::Color(res[0] * 255,res[1] * 255, res[2] * 255, 255));
				}
				newPos += direction;
			}
		}
		void EightWaySymmetricSetColor(const sf::Vector2f& origin, const sf::Vector2f& r, Petals& petals, EvoAI::NeuralNetwork& nn, int currentRadius, int currentLayer) noexcept{
			setColorAndCut(sf::Vector2f(r.x + origin.x, r.y + origin.y), nn, petals, currentRadius, currentLayer);
			setColorAndCut(sf::Vector2f(r.x + origin.x, -r.y + origin.y), nn, petals, currentRadius, currentLayer);
			setColorAndCut(sf::Vector2f(-r.x + origin.x, -r.y + origin.y), nn, petals, currentRadius, currentLayer);
			setColorAndCut(sf::Vector2f(-r.x + origin.x, r.y + origin.y), nn, petals, currentRadius, currentLayer);

			setColorAndCut(sf::Vector2f(r.y + origin.x, r.x + origin.y), nn, petals, currentRadius, currentLayer);
			setColorAndCut(sf::Vector2f(r.y + origin.x, -r.x + origin.y), nn, petals, currentRadius, currentLayer);
			setColorAndCut(sf::Vector2f(-r.y + origin.x, -r.x + origin.y), nn, petals, currentRadius, currentLayer);
			setColorAndCut(sf::Vector2f(-r.y + origin.x, r.x + origin.y), nn, petals, currentRadius, currentLayer);
		}
		void drawPetals(Petals& petals, EvoAI::NeuralNetwork& nn,int currentRadius, int currentLayer) noexcept{
			int x = 0;
			int y = currentRadius;
			int d = 1-y;
			const auto& origin = sf::Vector2f(petals.radius,petals.radius);
			EightWaySymmetricSetColor(origin,sf::Vector2f(x,y),petals,nn,currentRadius,currentLayer);
			while(x<=y){
				if(d<=0){
					d -= 2*x+1;
				}else{
					d += 2*y+1;
					--y;
				}
				++x;
				EightWaySymmetricSetColor(origin,sf::Vector2f(x,y),petals,nn,currentRadius,currentLayer);
			}
		}
		void drawTrunk(Petals& petals) noexcept{
			const auto& origin = sf::Vector2f(petals.radius,petals.radius);
			const auto& size = petals.image.getSize();
			auto mask = sf::Color::Black;
			mask.a = 128;
			for(auto y=origin.y;y<size.y;++y){
				petals.image.setPixel(origin.x-1,y,sf::Color::Green + mask);
				petals.image.setPixel(origin.x,y,sf::Color::Green + mask);
				petals.image.setPixel(origin.x+1,y,sf::Color::Green + mask);
			}
		}
	}//priv/
	void draw(Petals::Type t, Petals& petals, EvoAI::Genome& g) noexcept{
		switch(t){
			case Petals::Type::Trunk:
				priv::drawTrunk(petals);
				break;
			case Petals::Type::Petals:
				{
					auto nn = EvoAI::Genome::makePhenotype(g);
					auto r = petals.radius;
					for(auto layer = petals.numLayers;layer>=0;--layer){
						priv::drawPetals(petals,nn,r,layer);
						r /= 2;
					}
				}break;
			case Petals::Type::TrunkAndPetals:
				{
					priv::drawTrunk(petals);
					auto nn = EvoAI::Genome::makePhenotype(g);
					auto r = petals.radius;
					for(auto layer = petals.numLayers;layer>=0;--layer){
						priv::drawPetals(petals,nn,r,layer);
						r /= 2;
					}
				}break;
		}
	}
}
