#include "AnalyticComplexChooserEngine.h"
#include <boost\math\distributions.hpp>
//#include <ql/pricingengines/blackscholescalculator.hpp>
//#include <ql/math/distributions/bivariatenormaldistribution.hpp>
#include <ql/quantlib.hpp>

namespace QuantLib {

	AnalyticComplexChooserEngine::AnalyticComplexChooserEngine(
		const boost::shared_ptr<GeneralizedBlackScholesProcess>& process)
		: process_(process) {
			registerWith(process_);
	}


	AnalyticComplexChooserEngine::~AnalyticComplexChooserEngine()
	{
		unregisterWithAll();
	}

	void AnalyticComplexChooserEngine::calculate() const {
		results_.value=ComplexChooser();
	}

	Real AnalyticComplexChooserEngine::ComplexChooser() const{
		Real S = process_->x0();
		Real b;
		Real v;
		Real r = riskFreeRate(choosingDate());
		Real Xc = arguments_.strikeCall;
		Real Xp = arguments_.strikePut;
		Time Tc = callMaturity();
		Time Tp = putMaturity();
		Time T = choosingDate();

		Real i = CriticalValueChooser();
		std::cout << "Critical value :" << i << std::endl;
		i = 51.1158;

		b=riskFreeRate(choosingDate()) - dividendYield(choosingDate());
		v = volatility(T);
		Real d1 = (log(S / i) + (b + pow(v, 2) / 2)*T) / (v*sqrt(T));
		Real d2 = d1 - v*sqrt(T);

		b=riskFreeRate(callMaturity()) - dividendYield(callMaturity());
		v = volatility(Tc);
		Real y1 = (log(S / Xc) + (b + pow(v, 2) / 2)*Tc) / (v*sqrt(Tc));

		b=riskFreeRate(putMaturity()) - dividendYield(putMaturity());
		v = volatility(Tp);
		Real y2 = (log(S / Xp) + (b + pow(v, 2) / 2)*Tp) / (v*sqrt(Tp));

		Real rho1 = sqrt(T / Tc);
		Real rho2 = sqrt(T / Tp);
		b=riskFreeRate(callMaturity()) - dividendYield(callMaturity());
		r = riskFreeRate(callMaturity());
		Real ComplexChooser = S * exp((b - r)*Tc) *  BivariateCumulativeNormalDistributionDr78(rho1)(d1, y1) 
			- Xc * exp(-r*Tc)*BivariateCumulativeNormalDistributionDr78(rho1)(d2, y1 - v * sqrt(Tc)) ;
		b=riskFreeRate(putMaturity()) - dividendYield(putMaturity());
		r = riskFreeRate(putMaturity());
		ComplexChooser-= S * exp((b - r)*Tp) * BivariateCumulativeNormalDistributionDr78(rho2)(-d1, -y2);
		ComplexChooser+= Xp * exp(-r*Tp) * BivariateCumulativeNormalDistributionDr78(rho2)(-d2, -y2 + v * sqrt(Tp));
		return ComplexChooser;
	}

	//GBlackSchole and GDelta Optimisation: 
	//Two in one function so it does not have to use
	//BlackScholesCalculator two times (value then delta )for the same option.
	BlackScholesCalculator AnalyticComplexChooserEngine::bsCalculator(Real spot, Option::Type optionType) const {
		//Real spot = process_->x0();
		Real vol;
		DiscountFactor growth;
		DiscountFactor discount;

		//payoff 
		boost::shared_ptr<PlainVanillaPayoff > vanillaPayoff;
		if (optionType == Option::Type::Call){
			//TC-T
			Time t=callMaturity()-choosingDate();
			//payoff for a Call Option
			vanillaPayoff = boost::shared_ptr<PlainVanillaPayoff>(new PlainVanillaPayoff(Option::Type::Call, strike(Option::Type::Call)));
			//QuantLib requires sigma * sqrt(T) rather than just sigma/volatility
			vol = volatility(t) * std::sqrt(t);
			//calculate dividend discount factor assuming continuous compounding (e^-rt)
			growth = dividendDiscount(t);
			//calculate payoff discount factor assuming continuous compounding 
			discount = riskFreeDiscount(t);
		}
		else{

			Time t=putMaturity()-choosingDate();			
			vanillaPayoff = boost::shared_ptr<PlainVanillaPayoff>(new PlainVanillaPayoff(Option::Type::Put, strike(Option::Type::Put)));
			vol = volatility(t) * std::sqrt(t);
			growth = dividendDiscount(t);
			discount = riskFreeDiscount(t);
		}

		BlackScholesCalculator bs(vanillaPayoff, spot, growth, vol, discount);
		return bs;
	}

	Real AnalyticComplexChooserEngine::CriticalValueChooser() const{
		Real Sv = process_->x0();

		BlackScholesCalculator bs=bsCalculator(Sv,Option::Type::Call);
		Real ci = bs.value();
		Real dc = bs.delta();

		bs=bsCalculator(Sv,Option::Type::Put);
		Real Pi = bs.value();
		Real dp = bs.delta();

		Real yi = ci - Pi;
		Real di = dc - dp;
		Real epsilon = 0.001;

		//Newton-Raphson prosess
		while (abs(yi) > epsilon){
			Sv = Sv - yi / di;

			bs=bsCalculator(Sv,Option::Type::Call);
			ci = bs.value();
			dc = bs.delta();

			bs=bsCalculator(Sv,Option::Type::Put);
			Pi = bs.value();
			dp = bs.delta();

			yi = ci - Pi;
			di = dc - dp;
		}
		return Sv;
	}

	//Real AnalyticComplexChooserEngine::underlying() const {
	//return process_->x0();
	//}


	Real AnalyticComplexChooserEngine::strike(Option::Type optionType) const {
		if (optionType == Option::Type::Call)
			return arguments_.strikeCall;
		else
			return arguments_.strikePut;
	}

	Time AnalyticComplexChooserEngine::choosingDate() const {
		return process_->time(arguments_.choosingDate);
	}
	Time AnalyticComplexChooserEngine::putMaturity() const {
		return process_->time(arguments_.exercisePut->lastDate());
	}
	Time AnalyticComplexChooserEngine::callMaturity() const {
		return process_->time(arguments_.exerciseCall->lastDate());
	}

	//Je ne sais pas pour le strike et je ne sais pas si il faut utiliser cette volatility dans GBlackScholes
	Volatility AnalyticComplexChooserEngine::volatility(Time t) const {
		return process_->blackVolatility()->blackVol(t, arguments_.strikeCall);
	}

	Rate AnalyticComplexChooserEngine::dividendYield(Time t) const {
		return process_->dividendYield()->zeroRate(t,
			Continuous, NoFrequency);
	}

	DiscountFactor AnalyticComplexChooserEngine::dividendDiscount(Time t) const {
		return process_->dividendYield()->discount(t);
	}

	Rate AnalyticComplexChooserEngine::riskFreeRate(Time t) const {
		return process_->riskFreeRate()->zeroRate(t, Continuous,
			NoFrequency);
	}

	DiscountFactor AnalyticComplexChooserEngine::riskFreeDiscount(Time t) const {
		return process_->riskFreeRate()->discount(t);
	}

}

