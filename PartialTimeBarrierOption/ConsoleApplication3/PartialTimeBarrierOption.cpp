#include "PartialTimeBarrierOption.h"
#include <ql/instruments/impliedvolatility.hpp>
#include <ql/pricingengines/barrier/analyticbarrierengine.hpp>
#include <ql/exercise.hpp>
#include <boost/scoped_ptr.hpp>

using namespace QuantLib;

PartialTimeBarrierOption::PartialTimeBarrierOption(PartialBarrier::Type barrierType,
												   Real barrier,
												   Real rebate,
												   Date coverEventDate,
												   const boost::shared_ptr<StrikedTypePayoff>& payoff,
												   const boost::shared_ptr<Exercise>& exercise)
												   : OneAssetOption(payoff, exercise),
												   barrierType_(barrierType), barrier_(barrier), rebate_(rebate), coverEventDate_(coverEventDate) {}

void PartialTimeBarrierOption::setupArguments(PricingEngine::arguments* args) const {

	OneAssetOption::setupArguments(args);

	PartialTimeBarrierOption::arguments* moreArgs =
		dynamic_cast<PartialTimeBarrierOption::arguments*>(args);
	QL_REQUIRE(moreArgs != 0, "wrong argument type");
	moreArgs->barrierType = barrierType_;
	moreArgs->barrier = barrier_;
	moreArgs->rebate = rebate_;
	moreArgs->coverEventDate = coverEventDate_;
}

PartialTimeBarrierOption::arguments::arguments()
	: barrierType(PartialBarrier::Type(-1)), barrier(Null<Real>()),
	rebate(Null<Real>()), coverEventDate(Null<Date>()) {}

void PartialTimeBarrierOption::arguments::validate() const {
	OneAssetOption::arguments::validate();

	switch (barrierType) {
	case PartialBarrier::DownInEnd:
	case PartialBarrier::DownInStart:
	case PartialBarrier::DownOutStart:
	case PartialBarrier::DownOutEndB1:		
	case PartialBarrier::DownOutEndB2:
	case PartialBarrier::UpInEnd:
	case PartialBarrier::UpInStart:
	case PartialBarrier::UpOutStart:
	case PartialBarrier::UpOutEndB1:
	case PartialBarrier::UpOutEndB2:
		break;
	default:
		QL_FAIL("unknown type");
	}

	QL_REQUIRE(coverEventDate >= exercise->lastDate(), "cover event date is greater than or equal to expiry date");
	QL_REQUIRE(barrier != Null<Real>(), "no barrier given");
	QL_REQUIRE(rebate != Null<Real>(), "no rebate given");
	QL_REQUIRE(coverEventDate != Null<Date>(), "no cover event date given");
}

bool PartialTimeBarrierOption::engine::triggered(Real underlying) const {
	switch (arguments_.barrierType) {
	case PartialBarrier::DownInEnd:
	case PartialBarrier::DownInStart:
	case PartialBarrier::DownOutStart:
	case PartialBarrier::DownOutEndB2:
		return underlying < arguments_.barrier;
	case PartialBarrier::DownOutEndB1:
		return underlying <= arguments_.barrier;
	case PartialBarrier::UpInEnd:
	case PartialBarrier::UpInStart:
	case PartialBarrier::UpOutStart:
	case PartialBarrier::UpOutEndB2:
		return underlying > arguments_.barrier;
	case PartialBarrier::UpOutEndB1:
		return underlying >= arguments_.barrier;
	default:
		QL_FAIL("unknown type");
	}
}