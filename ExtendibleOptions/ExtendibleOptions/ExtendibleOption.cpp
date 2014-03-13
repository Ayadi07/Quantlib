#include <ql/quantlib.hpp>
#include "ExtendibleOption.h"

namespace QuantLib {
	ExtendibleOption::ExtendibleOption(Option::Type type,
		ExtendibleOption::Type writerHolder,
		Real premium,
		Date secondExpiryDate,
		Real secondStrike,
		const boost::shared_ptr<StrikedTypePayoff>& payoff,
		const boost::shared_ptr<Exercise>& exercise):OneAssetOption(payoff,exercise),
		writerHolder_(writerHolder),
		premium_(premium),
		secondExpiryDate_(secondExpiryDate),
		secondStrike_(secondStrike)
	{
	}
	void ExtendibleOption::setupArguments(
		PricingEngine::arguments* args) const {
			OneAssetOption::setupArguments(args);
			ExtendibleOption::arguments* moreArgs =
				dynamic_cast<ExtendibleOption::arguments*>(args);
			QL_REQUIRE(moreArgs != 0, "wrong argument type");
			moreArgs->writerHolder=writerHolder_;
			moreArgs->premium=premium_;
			moreArgs->secondExpiryDate=secondExpiryDate_;
			moreArgs->secondStrike=secondStrike_;
	}

	void ExtendibleOption::	arguments::validate() const {
		OneAssetOption::arguments::validate();
		QL_REQUIRE(premium > 0,"negative premium not allowed");
		QL_REQUIRE(secondExpiryDate != Date() , "no extending date given");
		QL_REQUIRE(secondExpiryDate >= exercise->lastDate(),"extending date is earlier than or equal to first maturity date");

	}

	ExtendibleOption::~ExtendibleOption(void)
	{
	}
}