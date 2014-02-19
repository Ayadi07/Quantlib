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

		Real underlying() const;
		Real strike(Option::Type optionType) const;
		Time residualTime() const;
		Time volatility() const;
		Rate dividendYield() const;
		DiscountFactor dividendDiscount() const;
		Rate riskFreeRate() const;
		DiscountFactor riskFreeDiscount() const;
		Real GBlackScholes(Option::Type optionType) const;
		Real GDelta(Option::Type optionType) const;

		
	};
}

