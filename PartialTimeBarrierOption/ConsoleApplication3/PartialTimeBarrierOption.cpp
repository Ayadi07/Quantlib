#include "PartialTimeBarrierOption.h"
#include <ql/instruments/impliedvolatility.hpp>
#include <ql/pricingengines/barrier/analyticbarrierengine.hpp>
#include <ql/exercise.hpp>
#include <boost/scoped_ptr.hpp>

using namespace QuantLib;

PartialTimeBarrierOption::PartialTimeBarrierOption(PartialBarrier::Type barrierType,
												   PartialBarrier::Range barrierRange,
												   Real barrier,
												   Real rebate,
												   Date coverEventDate,
												   const boost::shared_ptr<StrikedTypePayoff>& payoff,
												   const boost::shared_ptr<Exercise>& exercise)
												   : OneAssetOption(payoff, exercise),
												   barrierType_(barrierType), barrierRange_(barrierRange),barrier_(barrier), rebate_(rebate), coverEventDate_(coverEventDate) {}

void PartialTimeBarrierOption::setupArguments(PricingEngine::arguments* args) const {

	OneAssetOption::setupArguments(args);

	PartialTimeBarrierOption::arguments* moreArgs =
		dynamic_cast<PartialTimeBarrierOption::arguments*>(args);
	QL_REQUIRE(moreArgs != 0, "wrong argument type");
	moreArgs->barrierType = barrierType_;
	moreArgs->barrierRange = barrierRange_;
	moreArgs->barrier = barrier_;
	moreArgs->rebate = rebate_;
	moreArgs->coverEventDate = coverEventDate_;
}

PartialTimeBarrierOption::arguments::arguments()
	: barrierType(PartialBarrier::Type(-1)),barrierRange(PartialBarrier::Range(-1)), barrier(Null<Real>()),
	rebate(Null<Real>()), coverEventDate(Null<Date>()) {}

void PartialTimeBarrierOption::arguments::validate() const {
	OneAssetOption::arguments::validate();

	// barrier variants{ DownInStart, DownInEnd, UpInStart, UpInEnd, DownOutStart,DownOutEndB1, DownOutEndB2, UpOutStart,UpOutEndB1,UpOutEndB2 };
	//checking barrier type
	switch (barrierType) {
	case PartialBarrier::DownIn:
		//checking suitable barrier range
		switch (barrierRange)
		{
		case PartialBarrier::Range::End:
		case PartialBarrier::Range::Start:
			break;
		default:
			QL_FAIL("DownIn barrier comes only with End and Start range!");
		} break;
	case PartialBarrier::DownOut:
		//checking suitable barrier range
		switch (barrierRange)
		{
		case PartialBarrier::Range::End:
			QL_FAIL("DownOut barrier does not come with End range!");
			break;
		default:;
		} break;
	case PartialBarrier::UpIn:
		//checking suitable barrier range
		switch (barrierRange)
		{
		case PartialBarrier::Range::End:
		case PartialBarrier::Range::Start:
			break;
		default:
			QL_FAIL("UpIn barrier comes only with End and Start range!");
		} break;
	case PartialBarrier::UpOut:
		//checking suitable barrier range
		switch (barrierRange)
		{
		case PartialBarrier::Range::End:
			QL_FAIL("UpOut barrier does not come with End range!");
			break;
		default:;
		}break;
	default:
		QL_FAIL("unknown barrier type");
	}
	//end checking barrier type

	//checking barrier range apart from barrier type
	switch (barrierRange) {
	case PartialBarrier::Range::End:
	case PartialBarrier::Range::EndB1:
	case PartialBarrier::Range::EndB2:
	case PartialBarrier::Range::Start:
		break;
	default:
		QL_FAIL("unknown barrier range");
	}
	//end checking barrier range

	////////////////Je ne sais pas encore comment en peu comparer "coverEventDate" avec le "settlement"
	////////////////il est important de savoir si on a entrer une date avant la vie de l'option.
	/*ca ne marche pas!	
	if((coverEventDate!=Null<Date>())&&(coverEventDate>=exercise->date))
	{
	QL_FAIL("Cover Event Date is greater than expiracy date");
	}
	*/
	////////////////Je ne sais pas encore comment en peu comparer "coverEventDate" avec le "settlement"
	////////////////il est important de savoir si on a entrer une date avant la vie de l'option.

	QL_REQUIRE(barrier != Null<Real>(), "no barrier given");
	QL_REQUIRE(rebate != Null<Real>(), "no rebate given");
	QL_REQUIRE(coverEventDate != Null<Date>(), "no cover event date given");
}

bool PartialTimeBarrierOption::engine::triggered(Real underlying) const {
	switch (arguments_.barrierType) {
	case PartialBarrier::Type::DownIn:
	case PartialBarrier::DownOut:
		switch (arguments_.barrierRange)
		{
		case PartialBarrier::Range::EndB1:
			return underlying <= arguments_.barrier;break;
		default:
			return underlying < arguments_.barrier;
			break;
		};
	case PartialBarrier::Type::UpIn:
	case PartialBarrier::Type::UpOut:
		switch (arguments_.barrierRange)
		{
		case PartialBarrier::Range::EndB1:
			return underlying >= arguments_.barrier;
		default:
			return underlying > arguments_.barrier;
			break;
		};
	default:
		QL_FAIL("unknown type");
	}
}