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
                Real underlying() const;
                Real strike() const;
                Time residualTime() const;
                Time coverEventTime() const;
                Volatility volatility(int i) const;
                Real barrier() const;
                Real rebate() const;
                Real stdDeviation() const;
                Rate riskFreeRate() const;
                DiscountFactor riskFreeDiscount() const;
                Rate dividendYield() const;
                DiscountFactor dividendDiscount() const;
                Real M(Real a,Real b,Real rho) const;
                Real d1()const;
                Real d2()const;
                Real e1() const;
                Real e2() const;
                Real e3() const;
                Real e4() const;
                Real f1() const;
                Real f2() const;
                Real rho() const;
                Rate mu() const;
                Real CoB2(PartialBarrier::Type barrierType) const;
                Real CoB1() const;
                Real CA(Integer n) const;
				Real CIA(Integer n) const;
                Real g1()const;
                Real g2()const;
                Real g3()const;
                Real g4()const;
				Real LOG(Real r) const;
				Real SQRT(Real r) const;
				Real HS(Real S, Real H, Real power)const;
        };
}