#include <ql/instruments/oneassetoption.hpp>
#include <ql/instruments/barriertype.hpp>
#include <ql/instruments/payoffs.hpp>

namespace QuantLib {
	struct PartialBarrier:public Barrier
	{
		
		enum Range { Start, End, EndB1,EndB2};
	};

	class GeneralizedBlackScholesProcess;
	class PartialTimeBarrierOption : public OneAssetOption {
	public:
		class arguments;
		class engine;
		//PartialTimeBarrierOption(PartialBarrier::Type barrierType,
		PartialTimeBarrierOption(PartialBarrier::Type barrierType,
			PartialBarrier::Range barrierRange,
			Real barrier,
			Real rebate,
			Date coverEventDate,
			const boost::shared_ptr<StrikedTypePayoff>& payoff,
			const boost::shared_ptr<Exercise>& exercise);
		void setupArguments(PricingEngine::arguments*) const;
	protected:
		PartialBarrier::Type barrierType_;
		PartialBarrier::Range barrierRange_;
		Real barrier_;
		Real rebate_;
		Date coverEventDate_;
	};

	//! %Arguments for barrier option calculation
	class PartialTimeBarrierOption::arguments : public OneAssetOption::arguments {
	public:
		arguments();
		PartialBarrier::Type barrierType;
		PartialBarrier::Range barrierRange;
		Real barrier;
		Real rebate;
		Date coverEventDate;
		void validate() const;
	};

	//! %Partial-Time-Barrier-Option %engine base class
	class PartialTimeBarrierOption::engine
		: public GenericEngine<PartialTimeBarrierOption::arguments,
		PartialTimeBarrierOption::results> {
	protected:
		bool triggered(Real underlying) const;
	};	
	
}