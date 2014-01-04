#include <ql/instruments/oneassetoption.hpp>
#include <ql/instruments/barriertype.hpp>
#include <ql/instruments/payoffs.hpp>

namespace QuantLib {
	class GeneralizedBlackScholesProcess;
	class PartialTimeBarrierOption : public OneAssetOption {
	public:
		class arguments;
		class engine;
		PartialTimeBarrierOption(Barrier::Type barrierType,
			Real barrier,
			Real rebate,
			Date coverEventDate,
			const boost::shared_ptr<StrikedTypePayoff>& payoff,
			const boost::shared_ptr<Exercise>& exercise);
		void setupArguments(PricingEngine::arguments*) const;

		Volatility impliedVolatility(
			Real price,
			const boost::shared_ptr<GeneralizedBlackScholesProcess>& process,
			Real accuracy = 1.0e-4,
			Size maxEvaluations = 100,
			Volatility minVol = 1.0e-7,
			Volatility maxVol = 4.0) const;
	protected:
		Barrier::Type barrierType_;
		Real barrier_;
		Real rebate_;
		Date coverEventDate_;
	};

	//! %Arguments for barrier option calculation
	class PartialTimeBarrierOption::arguments : public OneAssetOption::arguments {
	public:
		arguments();
		Barrier::Type barrierType;
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