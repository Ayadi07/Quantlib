#include <iostream>
#include "AnalyticComplexChooserEngine.h"
#include <ql/time/daycounter.hpp>
#include <ql/time/calendar.hpp>
#include <ql/time/calendars/target.hpp>
#include <ql/settings.hpp>
#include <ql/time/daycounters/actual360.hpp>
#include <ql/exercise.hpp>
#include <ql\quotes\simplequote.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>
#include <ql\termstructures\volatility\equityfx\blackconstantvol.hpp>
#include <ql/termstructures/yield/flatforward.hpp>
#include <ql/processes/blackscholesprocess.hpp>

using namespace QuantLib;
int main(int, char*[])
{
	std::cout << std::endl;
	// set up dates
	Calendar calendar = TARGET();
	Date todaysDate(3, February, 2014);
	Date settlementDate(5, February, 2014);
	Settings::instance().evaluationDate() = todaysDate;
	Date choosingDate(settlementDate + 90);
	Date maturityCall(choosingDate + 180);
	Date maturityPut(choosingDate + 210);
	DayCounter dayCounter = Actual360();

	//Option parameter
	Real underlying = 50;
	Real strikeCall = 55;
	Real strikePut = 48;
	Spread dividendYield = 0.05;
	Rate riskFreeRate = 0.1;
	Volatility volatility = 0.35;

	//basic option Call
	boost::shared_ptr<Exercise> exerciseCall(new EuropeanExercise(maturityCall));

	//basic option Put
	boost::shared_ptr<Exercise> exercisePut(new EuropeanExercise(maturityPut));

	ComplexChooserOption complexChooserOption(choosingDate, strikeCall, strikePut, exerciseCall, exercisePut);

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

	complexChooserOption.setPricingEngine(boost::shared_ptr<PricingEngine>(
		new AnalyticComplexChooserEngine(bsmProcess)));

	std::cout << "------------------------------------------------------"<<std::endl;
	std::cout << "S:" << underlying<< std::endl;
	std::cout << "Choosing date          : " << choosingDate<< std::endl;
	std::cout << "Expiry date for Call Tc: " << maturityCall<< std::endl;
	std::cout << "Expiry date for Put  Tp: " << maturityPut<< std::endl;
	std::cout << "Strike for Call      Xc: " << strikeCall<< std::endl;
	std::cout << "Strike for Put       Xp: " << strikePut<< std::endl;
	std::cout << "dividend              d: " << dividendYield<< std::endl;
	std::cout << "risk free rate        r: " << riskFreeRate<< std::endl;
	std::cout << "Volatility          vol: " << volatility<< std::endl;
	std::cout << std::endl;
	std::cout << "------------------------------------------------------"<<std::endl;
	std::cout << " *_Intermediate variables values_* "<<std::endl;
	
	std::cout << " Complex Chooser Option: " << complexChooserOption.NPV() << std::endl;

	std::cin.get();
	return 0;

}