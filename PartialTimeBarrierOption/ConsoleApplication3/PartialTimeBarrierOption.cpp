#include "PartialTimeBarrierOption.h"
#include <ql/instruments/impliedvolatility.hpp>
////////////////On va implementer la solution analytique sur les pappiers 
////////////////dans une classe AnalyticPartialTimeBarrierEngine
#include <ql/pricingengines/barrier/analyticbarrierengine.hpp>
////////////////On va implementer la solution analytique sur les pappiers 
////////////////dans une classe AnalyticPartialTimeBarrierEngine
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


Volatility PartialTimeBarrierOption::impliedVolatility(
	Real targetValue,
	const boost::shared_ptr<GeneralizedBlackScholesProcess>& process,
	Real accuracy,
	Size maxEvaluations,
	Volatility minVol,
	Volatility maxVol) const {

		QL_REQUIRE(!isExpired(), "option expired");

		boost::shared_ptr<SimpleQuote> volQuote(new SimpleQuote);

		boost::shared_ptr<GeneralizedBlackScholesProcess> newProcess =
			detail::ImpliedVolatilityHelper::clone(process, volQuote);


		boost::scoped_ptr<PricingEngine> engine;
		switch (exercise_->type()) {
		case Exercise::European:
			////////////////On va implementer la solution analytique sur les pappiers 
			////////////////dans une classe AnalyticPartialTimeBarrierEngine
			engine.reset(new AnalyticBarrierEngine(newProcess));
			////////////////On va implementer la solution analytique sur les pappiers 
			////////////////dans une classe AnalyticPartialTimeBarrierEngine
			break;
		case Exercise::American:
		case Exercise::Bermudan:
			QL_FAIL("engine not available for non-European partial-time barrier option");
			break;
		default:
			QL_FAIL("unknown exercise type");
		}

		return detail::ImpliedVolatilityHelper::calculate(*this,
			*engine,
			*volQuote,
			targetValue,
			accuracy,
			maxEvaluations,
			minVol, maxVol);
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