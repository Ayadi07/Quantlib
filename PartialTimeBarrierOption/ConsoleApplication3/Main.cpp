#include <iostream>
#include "AnalyticPartialTimeBarrierEngine.h"
#include <ql\time\calendars\target.hpp>
#include <ql\termstructures\yield\flatforward.hpp>
#include <ql\exercise.hpp>
#include <ql\quotes\simplequote.hpp>

#include <ql\termstructures\volatility\equityfx\blackconstantvol.hpp>




using namespace QuantLib;
int main(int, char*[])
{
	std::cout << std::endl;
	// set up dates
	Calendar calendar = TARGET();
	Date todaysDate(25, December, 2013);
	Settings::instance().evaluationDate() = todaysDate;
	Date settlementDate(27, December, 2013);
	Date maturity(27, December, 2014);
	Date coverEventDate(27, June, 2014);
	DayCounter dayCounter = Actual365Fixed();

	//Option parameter
	Option::Type type(Option::Call);
	Real underlying = 95;
	Real strike = 90;
	Spread dividendYield = 0.1;
	Rate riskFreeRate = 0.1;
	Volatility volatility = 0.25;

	//basic option
	boost::shared_ptr<StrikedTypePayoff> payoff(new PlainVanillaPayoff(type, strike));
	boost::shared_ptr<Exercise> europeanExercise(new EuropeanExercise(maturity));
	PartialTimeBarrierOption partialTimeBarrierOption(PartialBarrier::Type::DownOutEndB1,
												   100,
												   0.0,
												   coverEventDate,
												    payoff,
												   europeanExercise);
	


	//Handle setups
	Handle<Quote> underlyingH(boost::shared_ptr<Quote>(new SimpleQuote(underlying)));

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

	std::cout << "European Option value " << partialTimeBarrierOption.NPV() << std::endl;
	std::cin.get();
	return 0;
}