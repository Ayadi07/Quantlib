
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
		Time coverEventDate() const;
		Volatility volatility() const;
		Real barrier() const;
		Real rebate() const;
		Real stdDeviation() const;
		Rate riskFreeRate() const;
		DiscountFactor riskFreeDiscount() const;
		Rate dividendYield() const;
		DiscountFactor dividendDiscount() const;
		Real M(Real a,Real b,Real ro,Integer n) const;
		Real d1()const;
		Real d2()const;
		Real e1() const;
		Real e2() const;
		Real e3() const;
		Real e4() const;
		Real f1() const;
		Real f2() const;
		Real ro() const;
		Rate mu() const;
		Real CA(Real strike,Integer n) const;
	};
}