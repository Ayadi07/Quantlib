#include <ql/processes/blackscholesprocess.hpp>
#include <ql/math/distributions/normaldistribution.hpp>
#include "ExtendibleOption.h"

namespace QuantLib {

	class AnalyticExtendibleEngine : public ExtendibleOption::engine
	{
	public:
		AnalyticExtendibleEngine(
			const boost::shared_ptr<GeneralizedBlackScholesProcess>& process);
		~AnalyticExtendibleEngine();
		void calculate() const;

	private:
		boost::shared_ptr<GeneralizedBlackScholesProcess> process_;
		Real strike() const;
		Time firstExpiryTime() const;
		Time SecondExpiryTime() const;
		Volatility volatility() const;
		Rate riskFreeRate() const;
		Rate dividendYield() const;
		DiscountFactor dividendDiscount(Time t) const;
		DiscountFactor riskFreeDiscount(Time t) const;
		Real I1Call() const;
		Real I2Call() const;
		Real I1Put() const;
		Real I2Put() const;
		BlackScholesCalculator bsCalculator(Real spot, Option::Type optionType) const;
		Real M2(Real a, Real b, Real c, Real d, Real rho) const;
		Real N2(Real a, Real b) const;
		Real y1(Option::Type) const;
		Real y2(Option::Type) const;
		Real z1() const;
		Real z2() const;
	};
}

