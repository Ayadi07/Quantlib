#include "AnalyticComplexChooserEngine.h"
#include <boost\math\distributions.hpp>
#include <ql/pricingengines/blackscholescalculator.hpp>


namespace QuantLib {

	AnalyticComplexChooserEngine::AnalyticComplexChooserEngine(
		const boost::shared_ptr<GeneralizedBlackScholesProcess>& process)
		: process_(process) {
		registerWith(process_);
	}


	AnalyticComplexChooserEngine::~AnalyticComplexChooserEngine()
	{
	}

	void AnalyticComplexChooserEngine::calculate() const {
		
	}

	Real AnalyticComplexChooserEngine::GBlackScholes(Option::Type optionType) const{
		Real spot = process_->x0();
		//QuantLib requires sigma * sqrt(T) rather than just sigma/volatility
		Real vol = volatility() * std::sqrt(residualTime());
		//calculate dividend discount factor assuming continuous compounding (e^-rt)
		DiscountFactor growth = dividendDiscount();

		//calculate payoff discount factor assuming continuous compounding 
		DiscountFactor discount = riskFreeDiscount();

		//instantiate payoff function for a call 
		boost::shared_ptr<PlainVanillaPayoff > vanillaCallPayoff;
		if (optionType == Option::Type::Call){
			
			vanillaCallPayoff = boost::shared_ptr<PlainVanillaPayoff>(new PlainVanillaPayoff(Option::Type::Call, strike(Option::Type::Call)));
		}
		else{
			vanillaCallPayoff = boost::shared_ptr<PlainVanillaPayoff>(new PlainVanillaPayoff(Option::Type::Put, strike(Option::Type::Put)));
		}

		BlackScholesCalculator bsCalculator(vanillaCallPayoff, spot, growth, vol, discount);

		return bsCalculator.value();

	}

	Real AnalyticComplexChooserEngine::GDelta(Option::Type optionType) const{
		Real spot = process_->x0();
		//QuantLib requires sigma * sqrt(T) rather than just sigma/volatility
		Real vol = volatility() * std::sqrt(residualTime());
		//calculate dividend discount factor assuming continuous compounding (e^-rt)
		DiscountFactor growth = dividendDiscount();

		//calculate payoff discount factor assuming continuous compounding 
		DiscountFactor discount = riskFreeDiscount();

		//instantiate payoff function for a call 
		boost::shared_ptr<PlainVanillaPayoff > vanillaCallPayoff;
		if (optionType == Option::Type::Call){

			vanillaCallPayoff = boost::shared_ptr<PlainVanillaPayoff>(new PlainVanillaPayoff(Option::Type::Call, strike(Option::Type::Call)));
		}
		else{
			vanillaCallPayoff = boost::shared_ptr<PlainVanillaPayoff>(new PlainVanillaPayoff(Option::Type::Put, strike(Option::Type::Put)));
		}

		BlackScholesCalculator bsCalculator(vanillaCallPayoff, spot, growth, vol, discount);

		return bsCalculator.delta();
	}

	Real AnalyticComplexChooserEngine::underlying() const {
		return process_->x0();
	}

	//Faire pour avoir le strike du Call ou du Put
	Real AnalyticComplexChooserEngine::strike(Option::Type optionType) const {
		if (optionType == Option::Type::Call)
			return arguments_.strikeCall;
		else
			return arguments_.strikePut;
	}

	Time AnalyticComplexChooserEngine::residualTime() const {
		return process_->time(arguments_.exerciseCall->lastDate());
	}

	//Je ne sais pas pour le strike et je ne sais pas si il faut utiliser cette volatility dans GBlackScholes
	Volatility AnalyticComplexChooserEngine::volatility() const {
		return process_->blackVolatility()->blackVol(residualTime(), arguments_.strikeCall);
	}

	Rate AnalyticComplexChooserEngine::dividendYield() const {
		return process_->dividendYield()->zeroRate(residualTime(),
			Continuous, NoFrequency);
	}

	DiscountFactor AnalyticComplexChooserEngine::dividendDiscount() const {
		return process_->dividendYield()->discount(residualTime());
	}

	Rate AnalyticComplexChooserEngine::riskFreeRate() const {
		return process_->riskFreeRate()->zeroRate(residualTime(), Continuous,
			NoFrequency);
	}

	DiscountFactor AnalyticComplexChooserEngine::riskFreeDiscount() const {
		return process_->riskFreeRate()->discount(residualTime());
	}

}

