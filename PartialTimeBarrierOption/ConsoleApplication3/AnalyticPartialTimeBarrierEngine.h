
#include "PartialTimeBarrierOption.h"
#include <ql/instruments/barrieroption.hpp>
#include <ql/processes/blackscholesprocess.hpp>
#include <ql/math/distributions/normaldistribution.hpp>

namespace QuantLib {

	class AnalyticPartialTimeBarrierEngine : public PartialTimeBarrierOption::engine {
	public:
		AnalyticPartialTimeBarrierEngine(
			const boost::shared_ptr<GeneralizedBlackScholesProcess>& process);
		void calculate() const;
	private:
		boost::shared_ptr<GeneralizedBlackScholesProcess> process_;
		CumulativeNormalDistribution f_;
		Real underlying() const;
		Real strike() const;
		Time residualTime() const;
		Volatility volatility() const;
		Real barrier() const;
		Real rebate() const;
		Real stdDeviation() const;
		Rate riskFreeRate() const;
		DiscountFactor riskFreeDiscount() const;
		Rate dividendYield() const;
		DiscountFactor dividendDiscount() const;
		Rate mu() const;
		Real muSigma() const;
		Real A(Real phi) const;
		Real B(Real phi) const;
		Real C(Real eta, Real phi) const;
		Real D(Real eta, Real phi) const;
		Real E(Real eta) const;
		Real F(Real eta) const;
	};
}