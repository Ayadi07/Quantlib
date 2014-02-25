#include <iostream>
#include "AnalyticExtendibleEngine.h"
#include <ql\time\daycounters\actual360.hpp>
#include <ql\time\calendars\target.hpp>
#include <ql\exercise.hpp>
#include <ql\termstructures\yield\flatforward.hpp>
#include <ql\quotes\simplequote.hpp>
#include <ql\termstructures\volatility\equityfx\blackconstantvol.hpp>

using namespace QuantLib;
int main(int, char*[])
{
	Calendar calendar = TARGET();
	Date todaysDate(6, January, 2014);
	Settings::instance().evaluationDate() = todaysDate;
	Date settlementDate(8, January, 2014);
	Date maturity(settlementDate + 180);
	Date secondExpiryDate(settlementDate + 270);
	DayCounter dayCounter = Actual360();
	Real strike = 100;
	Real secondStrike = 105;

	Option::Type typeOpt(Option::Call);


	boost::shared_ptr<StrikedTypePayoff> payoff(new PlainVanillaPayoff(typeOpt, strike));
	boost::shared_ptr<Exercise> europeanExercise(new EuropeanExercise(maturity));
	
	Real premium = 1;
	Real underlying = 100;
	Real riskFreeRate = 0.08;
	Real dividendYield = 0.0;
	Real volatility = 0.25;

	ExtendibleOption extendedHolderCall(Option::Type::Call, 
		ExtendibleOptionType::Type::H, 
		premium, 
		secondExpiryDate, 
		secondStrike,
		payoff, 
		europeanExercise);

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

	extendedHolderCall.setPricingEngine(boost::shared_ptr<PricingEngine>(
		new AnalyticExtendibleEngine(bsmProcess)));

	std::cout << "\n\nExtendible Option : " << extendedHolderCall.NPV() << std::endl;

	std::cin.get();

	return 0;

}