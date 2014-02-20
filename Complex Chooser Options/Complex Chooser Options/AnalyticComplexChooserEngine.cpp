#include "AnalyticComplexChooserEngine.h"
#include <boost\math\distributions.hpp>
#include <ql/pricingengines/blackscholescalculator.hpp>
#include <ql/quantlib.hpp>


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
		//Je ne sais pas où intervient le b
		Real spot = process_->x0();
		//QuantLib requires sigma * sqrt(T) rather than just sigma/volatility
		//Prendre TC-T pour le residual time
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

	Real AnalyticComplexChooserEngine::ComplexChosser() const{
		Real S = process_->x0();
		Real b = riskFreeRate() - dividendYield();
		Real v = volatility();
		Real r = riskFreeRate();
		Real Xc = arguments_.strikeCall;
		Real Xp = arguments_.strikePut;
		Time Tc = process_->time(arguments_.choosingDateCall);
		Time Tp = process_->time(arguments_.choosingDatePut);
		Time T = residualTime();

		Real i = CriticalValueChooser();
		Real d1 = (log(S / i) + (b + pow(v, 2) / 2)*T) / (v*sqrt(T));
		Real d2 = d1 - v*sqrt(T);
		Real y1 = (log(S / Xc) + (b + pow(v, 2) / 2)*Tc) / (v*sqrt(Tc));
		Real y2 = (log(S / Xp) + (b + pow(v, 2) / 2)*Tp) / (v*sqrt(Tp));
		Real rho1 = sqrt(T / Tc);
		Real rho2 = sqrt(T / Tp);

		Real ComplexChooser = S * exp((b - r)*Tc) *  BivariateCumulativeNormalDistributionDr78(rho1)(d1, y1) 
			- Xc * exp(-r*Tc)*BivariateCumulativeNormalDistributionDr78(rho1)(d2, y1 - v * sqrt(Tc)) 
			- S * exp((b - r)*Tp) * BivariateCumulativeNormalDistributionDr78(rho2)(-d1, -y2) 
			+ Xp * exp(-r*Tp) * BivariateCumulativeNormalDistributionDr78(rho2)(-d2, -y2 + v * sqrt(Tp));
		return ComplexChooser;
	}

	Real AnalyticComplexChooserEngine::CriticalValueChooser() const{
		Real Sv = process_->x0();
		Real ci = GBlackScholes(Option::Type::Call);
		Real Pi = GBlackScholes(Option::Type::Put);
		Real dc = GDelta(Option::Type::Call);
		Real dp = GDelta(Option::Type::Put);
		Real yi = ci - Pi;
		Real di = dc - dp;
		Real epsilon = 0.001;

		//Newton-Raphson prosess
		while (abs(yi) > epsilon){
			Sv = Sv - yi / di;
			ci = GBlackScholes(Option::Type::Call);
			Pi = GBlackScholes(Option::Type::Put);
			dc = GDelta(Option::Type::Call);
			dp = GDelta(Option::Type::Put);
			yi = ci - Pi;
			di = dc - dp;
		}

		return Sv;
	}

	Real AnalyticComplexChooserEngine::underlying() const {
		return process_->x0();
	}

	
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

