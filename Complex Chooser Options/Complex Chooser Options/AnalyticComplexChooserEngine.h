#include <ql/processes/blackscholesprocess.hpp>
#include <ql/math/distributions/normaldistribution.hpp>
#include "ComplexChooserOption.h"
namespace QuantLib {
	class AnalyticComplexChooserEngine : public ComplexChooserOption::engine
	{
	public:
		AnalyticComplexChooserEngine(
			const boost::shared_ptr<GeneralizedBlackScholesProcess>& process);
		~AnalyticComplexChooserEngine();
		void calculate() const;

	private:

		boost::shared_ptr<GeneralizedBlackScholesProcess> process_;
		//Real underlying() const;
		Real strike(Option::Type optionType) const;
		Time choosingDate() const;
		Time putMaturity() const;
		Time callMaturity() const;
		Volatility volatility() const;

		Rate dividendYield(Time t) const;
		DiscountFactor dividendDiscount(Time t) const;

		Rate riskFreeRate(Time t) const;
		DiscountFactor riskFreeDiscount(Time t) const;
		BlackScholesCalculator bsCalculator(Real spot, Option::Type optionType) const;
		//Real GBlackScholes(Option::Type optionType) const;
		//Real GDelta(Option::Type optionType) const;
		Real CriticalValueChooser() const;

		Real ComplexChosser() const;
	};
}

