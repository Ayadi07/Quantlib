#include <iostream>
#include "AnalyticPartialTimeBarrierEngine.h"
#include <ql\time\calendars\target.hpp>
#include <ql\termstructures\yield\flatforward.hpp>
#include <ql\exercise.hpp>
#include <ql\quotes\simplequote.hpp>
#include <ql\time\daycounters\actual360.hpp>
#include <ql\termstructures\volatility\equityfx\blackconstantvol.hpp>

using namespace QuantLib;
int main(int, char*[])
{
	std::cout << std::endl;
	// set up dates
	Calendar calendar = TARGET();
	Date todaysDate(6, January, 2014);
	Settings::instance().evaluationDate() = todaysDate;
	Date settlementDate(8, January, 2014);
	Date maturity(settlementDate + 360);
	DayCounter dayCounter = Actual360();
		
	std::vector<Date> v_coverEventTime;
	//v_coverEventTime.push_back(settlementDate + 0);
	v_coverEventTime.push_back(settlementDate + 90);
	v_coverEventTime.push_back(settlementDate + 180);
	v_coverEventTime.push_back(settlementDate + 270);
	//v_coverEventTime.push_back(settlementDate + 360);
	
	Real u1 = 95;
	Real u2 = 105;
	std::vector<Real> v_underlying;
	v_underlying.push_back(u1);
	v_underlying.push_back(u1);
	v_underlying.push_back(u2);
	v_underlying.push_back(u2);

	Real s1 = 90;
	Real s2 = 110;
	std::vector<Real> v_strike;
	v_strike.push_back(s1);
	v_strike.push_back(s2);
	v_strike.push_back(s1);
	v_strike.push_back(s2);
	
	std::vector<Real> values;

	Option::Type type(Option::Call);
	Spread dividendYield = 0.0;
	Rate riskFreeRate = 0.1;
	Volatility volatility = 0.25;

	for(int i=0; i<v_strike.size(); i++){
		for(int j=0; j<v_coverEventTime.size(); j++){
			
			//basic option
			boost::shared_ptr<StrikedTypePayoff> payoff(new PlainVanillaPayoff(type, v_strike[i]));
			boost::shared_ptr<Exercise> europeanExercise(new EuropeanExercise(maturity));
			PartialTimeBarrierOption partialTimeBarrierOption(PartialBarrier::Type::DownOutEndB1,
				100.0,
				0.0,
				v_coverEventTime[j],
				payoff,
				europeanExercise);

			//Handle setups
			Handle<Quote> underlyingH(boost::shared_ptr<Quote>(new SimpleQuote(v_underlying[i])));

			Handle<YieldTermStructure> flatTermStructure(
				boost::shared_ptr<YieldTermStructure>(
				new FlatForward(settlementDate,
				riskFreeRate,
				dayCounter)));

			Handle<YieldTermStructure> flatDividendTS(
				boost::shared_ptr<YieldTermStructure>(
				new FlatForward(settlementDate,
				dividendYield,
				dayCounter)));

			Handle<BlackVolTermStructure> flatVolTS(
				boost::shared_ptr<BlackVolTermStructure>(
				new BlackConstantVol(settlementDate,
				calendar,
				volatility,
				dayCounter)));

			boost::shared_ptr<BlackScholesMertonProcess> bsmProcess(
				new BlackScholesMertonProcess(underlyingH,
				flatDividendTS,
				flatTermStructure,
				flatVolTS));

			partialTimeBarrierOption.setPricingEngine(boost::shared_ptr<PricingEngine>(
				new AnalyticPartialTimeBarrierEngine(bsmProcess)));

			std::cout << "Underlying : " << v_underlying[i]<< "  Strike : " << v_strike[i] << "  CoverEventTime : " << v_coverEventTime[j] << std::endl;
			std::cout << "Partial Time End Barrier Call Type B1 option " << partialTimeBarrierOption.NPV()<< std::endl;
		}
		std::cout << std::endl;
	}
	std::cin.get();
	return 0;
}