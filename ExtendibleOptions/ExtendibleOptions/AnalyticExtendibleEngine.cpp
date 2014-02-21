#include "AnalyticExtendibleEngine.h"
#include <ql/quantlib.hpp>
#include <ql/math/distributions/normaldistribution.hpp>


namespace QuantLib {

	AnalyticExtendibleEngine::AnalyticExtendibleEngine(
		const boost::shared_ptr<GeneralizedBlackScholesProcess>& process) : process_(process) {
		registerWith(process_);
	}


	AnalyticExtendibleEngine::~AnalyticExtendibleEngine()
	{
	}

	void AnalyticExtendibleEngine::calculate() const
	{
		//Récupérer le type : Holder / Writter
		//

		//Spot
		Real S = process_->x0;
		Real r = riskFreeRate();
		Real b = r - dividendYield();
		Real X1 = strike();
		Real X2 = arguments_.secondStrike;
		Time T2 = SecondExpiryTime();
		Time t1 = firstExpiryTime();

		//Premium
		Real A = arguments_.premium;

		Real y1;
		Real y2;
		Real z1 = this->z1();
		Real z2 = this->z2();
		Real rho = sqrt(t1 / T2);

		boost::shared_ptr<PlainVanillaPayoff> payoff =
			boost::dynamic_pointer_cast<PlainVanillaPayoff>(arguments_.payoff);


		//QuantLib requires sigma * sqrt(T) rather than just sigma/volatility
		Real vol = volatility() * std::sqrt(t1);

		//calculate dividend discount factor assuming continuous compounding (e^-rt)
		DiscountFactor growth = dividendDiscount(t1);

		//calculate payoff discount factor assuming continuous compounding 
		DiscountFactor discount = riskFreeDiscount(t1);

		Real result;

		if (payoff->optionType() == Option::Type::Call)
		{
			y1 = this->y1(Option::Type::Call);
			y2 = this->y2(Option::Type::Call);
			//instantiate payoff function for a call 
			boost::shared_ptr<PlainVanillaPayoff> vanillaCallPayoff =
				boost::shared_ptr<PlainVanillaPayoff>(new PlainVanillaPayoff(Option::Type::Call, X1));
			result = BlackScholesCalculator(vanillaCallPayoff, S, growth, vol, discount).value()
				+ S*exp((b - r)*T2)*M2(y1, y2, 0, z1, rho)
				- X2*exp(-r*T2)*M2(y1 - vol*sqrt(t1), y2 - vol*sqrt(t1), 0, z1 - vol*sqrt(T2), rho)
				- S*exp((b - r)*t1)*N2(y1, z2) + X1*exp(-r*t1)*N2(y1 - vol*sqrt(t1), z2 - vol*sqrt(t1))
				- A*exp(-r*t1)*N2(y1 - vol*sqrt(t1), y2 - vol*sqrt(t1));
		}
		else{
			y1 = this->y1(Option::Type::Put);
			y2 = this->y2(Option::Type::Put);
			//instantiate payoff function for a call 
			boost::shared_ptr<PlainVanillaPayoff> vanillaCallPayoff =
				boost::shared_ptr<PlainVanillaPayoff>(new PlainVanillaPayoff(Option::Type::Put, X1));
			result = BlackScholesCalculator(vanillaCallPayoff, S, growth, vol, discount).value()
				- S*exp((b - r)*T2)*M2(y1, y2, 0, -z1, rho)
				+ X2*exp(-r*T2)*M2(y1 - vol*sqrt(t1), y2 - vol*sqrt(t1), 0, -z1 + vol*sqrt(T2), rho)
				+ S*exp((b - r)*t1)*N2(z2, y2) - X1*exp(-r*t1)*N2(z2 - vol*sqrt(t1), y2 - vol*sqrt(t1))
				- A*exp(-r*t1)*N2(y1 - vol*sqrt(t1), y2 - vol*sqrt(t1));
		}

		this->results_.value = result;
	}

	Real AnalyticExtendibleEngine::I1Call() const{
		Real Sv = process_->x0();
		Real A = arguments_.premium;

		BlackScholesCalculator bs = bsCalculator(Sv, Option::Type::Call);
		Real ci = bs.value();
		Real dc = bs.delta();

		

		Real yi = ci - A;
		//da/ds = 0
		Real di = dc - 0;
		Real epsilon = 0.001;

		//Newton-Raphson prosess
		while (abs(yi) > epsilon){
			Sv = Sv - yi / di;

			bs = bsCalculator(Sv, Option::Type::Call);
			ci = bs.value();
			dc = bs.delta();

			yi = ci - A;
			di = dc - 0;
		}
		return Sv;
	}

	Real AnalyticExtendibleEngine::I2Call() const{
		Real Sv = process_->x0();
		Real X1 = strike();
		Real A = arguments_.premium;

		BlackScholesCalculator bs = bsCalculator(Sv, Option::Type::Call);
		Real ci = bs.value();
		Real dc = bs.delta();



		Real yi = ci - A - Sv + X1;
		//da/ds = 0
		Real di = dc - 0;
		Real epsilon = 0.001;

		//Newton-Raphson prosess
		while (abs(yi) > epsilon){
			Sv = Sv - yi / di;

			bs = bsCalculator(Sv, Option::Type::Call);
			ci = bs.value();
			dc = bs.delta();

			yi = ci - A - Sv + X1;
			di = dc - 0;
		}
		return Sv;
	}

	Real AnalyticExtendibleEngine::I1Put() const{
		Real Sv = process_->x0();
		//Srtike
		Real X1 = strike();
		//Premium
		Real A = arguments_.premium;

		BlackScholesCalculator bs = bsCalculator(Sv, Option::Type::Put);
		Real pi = bs.value();
		Real dc = bs.delta();



		Real yi = pi - A + Sv - X1;
		//da/ds = 0
		Real di = dc - 0;
		Real epsilon = 0.001;

		//Newton-Raphson prosess
		while (abs(yi) > epsilon){
			Sv = Sv - yi / di;

			bs = bsCalculator(Sv, Option::Type::Put);
			pi = bs.value();
			dc = bs.delta();

			yi = pi - A + Sv - X1;
			di = dc - 0;
		}
		return Sv;
	}

	Real AnalyticExtendibleEngine::I2Put() const{
		Real Sv = process_->x0();
		Real A = arguments_.premium;

		BlackScholesCalculator bs = bsCalculator(Sv, Option::Type::Put);
		Real pi = bs.value();
		Real dc = bs.delta();



		Real yi = pi - A;
		//da/ds = 0
		Real di = dc - 0;
		Real epsilon = 0.001;

		//Newton-Raphson prosess
		while (abs(yi) > epsilon){
			Sv = Sv - yi / di;

			bs = bsCalculator(Sv, Option::Type::Put);
			pi = bs.value();
			dc = bs.delta();

			yi = pi - A;
			di = dc - 0;
		}
		return Sv;
	}

	//GBlackSchole and GDelta Optimisation: 
	//Two in one function so it does not have to use
	//BlackScholesCalculator two times (value then delta )for the same option.
	BlackScholesCalculator AnalyticExtendibleEngine::bsCalculator(Real spot, Option::Type optionType) const {
		//Real spot = process_->x0();
		Real vol;
		DiscountFactor growth;
		DiscountFactor discount;
		Real X2 = arguments_.secondStrike;
		Time T2 = SecondExpiryTime();
		Time t1 = firstExpiryTime();
		Time t = T2 - t1;

		//payoff 
		boost::shared_ptr<PlainVanillaPayoff > vanillaPayoff;
		if (optionType == Option::Type::Call){
			//TC-T
			
			//payoff for a Call Option
			vanillaPayoff = boost::shared_ptr<PlainVanillaPayoff>(new PlainVanillaPayoff(Option::Type::Call, X2));
			
		}
		else{

			vanillaPayoff = boost::shared_ptr<PlainVanillaPayoff>(new PlainVanillaPayoff(Option::Type::Put, X2));

		}

		//QuantLib requires sigma * sqrt(T) rather than just sigma/volatility
		//
		// ça c'est du code que j'ai trouvé sur internet mais je sais pas pourquoi 
		//il faut multiplier par sqrt(t)
		//
		vol = volatility() * std::sqrt(t);
		//calculate dividend discount factor assuming continuous compounding (e^-rt)
		growth = dividendDiscount(t);
		//calculate payoff discount factor assuming continuous compounding 
		discount = riskFreeDiscount(t);

		BlackScholesCalculator bs(vanillaPayoff, spot, growth, vol, discount);
		return bs;
	}

	Real AnalyticExtendibleEngine::M2(Real a, Real b, Real c, Real d, Real rho) const
	{
		BivariateCumulativeNormalDistributionDr78 CmlNormDist(rho);
		return CmlNormDist(b, d) - CmlNormDist(a, d) - CmlNormDist(b, c) + CmlNormDist(a,c);
	}

	Real AnalyticExtendibleEngine::N2(Real a, Real b) const
	{
		NormalDistribution NormDist;
		return NormDist(b) - NormDist(a);
	}

	Real AnalyticExtendibleEngine::strike() const
	{
		boost::shared_ptr<PlainVanillaPayoff> payoff =
			boost::dynamic_pointer_cast<PlainVanillaPayoff>(arguments_.payoff);
		QL_REQUIRE(payoff, "non-plain payoff given");
		return payoff->strike();
	}

	Time AnalyticExtendibleEngine::firstExpiryTime() const
	{
		return process_->time(arguments_.exercise->lastDate());
	}

	Time AnalyticExtendibleEngine::SecondExpiryTime() const
	{
		return process_->time(arguments_.secondExpiryDate);
	}

	Volatility AnalyticExtendibleEngine::volatility() const 
	{
		return process_->blackVolatility()->blackVol(firstExpiryTime(), strike());
	}
	Rate AnalyticExtendibleEngine::riskFreeRate() const
	{
		return process_->riskFreeRate()->zeroRate(firstExpiryTime(), Continuous,
			NoFrequency);
	}
	Rate AnalyticExtendibleEngine::dividendYield() const
	{
		return process_->dividendYield()->zeroRate(firstExpiryTime(),
			Continuous, NoFrequency);
	}

	DiscountFactor AnalyticExtendibleEngine::dividendDiscount(Time t) const
	{
		return process_->dividendYield()->discount(t);
	}

	DiscountFactor AnalyticExtendibleEngine::riskFreeDiscount(Time t) const
	{
		return process_->riskFreeRate()->discount(t);
	}

	Real AnalyticExtendibleEngine::y1(Option::Type type) const
	{
		Real S = process_->x0();
		Real I2;
		if (type == Option::Type::Call)
			I2 = I2Call();
		else
			I2 = I2Put();

		Real b = riskFreeRate() - dividendYield();
		Real vol = volatility();
		Time t1 = firstExpiryTime();

		return (log(S / I2) + (b + pow(vol, 2) / 2)*t1) / (vol*sqrt(t1));
	}

	Real AnalyticExtendibleEngine::y2(Option::Type type) const
	{
		Real S = process_->x0();
		Real I1;
		if (type == Option::Type::Call)
			I1 = I1Call();
		else
			I1 = I1Put();

		Real b = riskFreeRate() - dividendYield();
		Real vol = volatility();
		Time t1 = firstExpiryTime();

		return (log(S / I1) + (b + pow(vol, 2) / 2)*t1) / (vol*sqrt(t1));
	}

	Real AnalyticExtendibleEngine::z1() const
	{
		Real S = process_->x0();
		Real X2 = arguments_.secondStrike;
		Real b = riskFreeRate() - dividendYield();
		Real vol = volatility();
		Time T2 = SecondExpiryTime();

		return (log(S / X2) + (b + pow(vol, 2) / 2)*T2) / (vol*sqrt(T2));
	}

	Real AnalyticExtendibleEngine::z2() const
	{
		Real S = process_->x0();
		Real X1 = strike();

		Real b = riskFreeRate() - dividendYield();
		Real vol = volatility();
		Time t1 = firstExpiryTime();

		return (log(S / X1) + (b + pow(vol, 2) / 2)*t1) / (vol*sqrt(t1));
	}
}
