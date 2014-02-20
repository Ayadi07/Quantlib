#include <ql/instruments/payoffs.hpp>
#include <ql/exercise.hpp>
#include "ComplexChooserOption.h"

namespace QuantLib {

    ComplexChooserOption::ComplexChooserOption(
                                  Date choosingDateCall,
								  Date choosingDatePut,
								  Real strikeCall,
								  Real strikePut,
								  const boost::shared_ptr<Exercise>& exerciseCall,
								  const boost::shared_ptr<Exercise>& exercisePut)
								  :OneAssetOption(boost::shared_ptr<Payoff>(
								  new PlainVanillaPayoff(Option::Call, strikeCall)),exerciseCall),
								  choosingDate_(choosingDate),
								  strikeCall_(strikeCall),
								  strikePut_(strikePut),
								  exerciseCall_(exerciseCall),
								  exercisePut_(exercisePut)
								  {}
	//Attention OneAssetOption(boost::shared_ptr<Payoff>(new PlainVanillaPayoff(Option::Call, strikeCall)),exerciseCall)
	//On a deux strike (call/put) il nous faut une solution pour ca
	//Je n'ai pas pu definir le constructeur de notre classe sans 
																//:OneAssetOption(boost::shared_ptr<Payoff>(
																//new PlainVanillaPayoff(Option::Call, strikeCall)),exerciseCall),
	//any ideas guys?
    void ComplexChooserOption::setupArguments(
                                    PricingEngine::arguments* args) const {
        OneAssetOption::setupArguments(args);
        ComplexChooserOption::arguments* moreArgs =
            dynamic_cast<ComplexChooserOption::arguments*>(args);
        QL_REQUIRE(moreArgs != 0, "wrong argument type");
        moreArgs->choosingDateCall=choosingDateCall_;
		moreArgs->choosingDatePut = choosingDatePut_;
		moreArgs->strikeCall=strikeCall_;
		moreArgs->strikePut=strikePut_;
    }

    void ComplexChooserOption::arguments::validate() const {
        OneAssetOption::arguments::validate();
		QL_REQUIRE(choosingDateCall != Date() || choosingDatePut != Date(), " no choosing date given");
		QL_REQUIRE(choosingDateCall < exerciseCall->lastDate(),
                   "choosing date later than or equal to Call maturity date");
		QL_REQUIRE(choosingDatePut < exercisePut->lastDate(),
                   "choosing date later than or equal to Put maturity date");
    }

}