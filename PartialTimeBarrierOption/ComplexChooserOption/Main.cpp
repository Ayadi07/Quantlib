#include <iostream>
#include "AnalyticComplexChooserEngine.h"
#include <ql\time\daycounters\actual360.hpp>
#include <ql\time\calendars\target.hpp>


using namespace QuantLib;
int main(int, char*[]) {
	Calendar calendar = TARGET();
	Date todaysDate(6, January, 2014);
	Settings::instance().evaluationDate() = todaysDate;
	Date settlementDate(8, January, 2014);
	Date choosingDate(settlementDate + 90);
	Date maturityCall(choosingDate + 180);
	Date maturityPut(choosingDate + 210);
	DayCounter dayCounter = Actual360();

	Real strikeCall = 55;
	Real strikePut = 48;

	
	Option::Type typeCall(Option::Call);
	Option::Type typePut(Option::Put);
	Spread dividendYield = 0.05;
	Rate riskFreeRate = 0.1;
	Volatility volatility = 0.35;

	//ComplexChooserOption cco(choosingDate,strikeCall, strikePut, 