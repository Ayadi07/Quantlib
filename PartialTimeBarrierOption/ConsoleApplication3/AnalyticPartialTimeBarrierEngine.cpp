#include "AnalyticPartialTimeBarrierEngine.h"
#include <ql/exercise.hpp>

namespace QuantLib {

	AnalyticPartialTimeBarrierEngine::AnalyticPartialTimeBarrierEngine(
		const boost::shared_ptr<GeneralizedBlackScholesProcess>& process)
		: process_(process) {
			registerWith(process_);
	}

	void AnalyticPartialTimeBarrierEngine::calculate() const {

		boost::shared_ptr<PlainVanillaPayoff> payoff =
			boost::dynamic_pointer_cast<PlainVanillaPayoff>(arguments_.payoff);
		QL_REQUIRE(payoff, "non-plain payoff given");
		QL_REQUIRE(payoff->strike()>0.0,
			"strike must be positive");

		Real strike = payoff->strike();
		Real spot = process_->x0();
		QL_REQUIRE(spot >= 0.0, "negative or null underlying given");
		QL_REQUIRE(!triggered(spot), "barrier touched");

		Barrier::Type barrierType = arguments_.barrierType;

		switch (payoff->optionType()) {
		case Option::Call:
			switch (barrierType) {
			case Barrier::DownIn:
				if (strike >= barrier())
					results_.value = C(1,1) + E(1);
				else
					results_.value = A(1) - B(1) + D(1,1) + E(1);
				break;
			case Barrier::UpIn:
				if (strike >= barrier())
					results_.value = A(1) + E(-1);
				else
					results_.value = B(1) - C(-1,1) + D(-1,1) + E(-1);
				break;
			case Barrier::DownOut:
				if (strike >= barrier())
					results_.value = A(1) - C(1,1) + F(1);
				else
					results_.value = B(1) - D(1,1) + F(1);
				break;
			case Barrier::UpOut:
				if (strike >= barrier())
					results_.value = F(-1);
				else
					results_.value = A(1) - B(1) + C(-1,1) - D(-1,1) + F(-1);
				break;
			}
			break;
		case Option::Put:
			switch (barrierType) {
			case Barrier::DownIn:
				if (strike >= barrier())
					results_.value = B(-1) - C(1,-1) + D(1,-1) + E(1);
				else
					results_.value = A(-1) + E(1);
				break;
			case Barrier::UpIn:
				if (strike >= barrier())
					results_.value = A(-1) - B(-1) + D(-1,-1) + E(-1);
				else
					results_.value = C(-1,-1) + E(-1);
				break;
			case Barrier::DownOut:
				if (strike >= barrier())
					results_.value = A(-1) - B(-1) + C(1,-1) - D(1,-1) + F(1);
				else
					results_.value = F(1);
				break;
			case Barrier::UpOut:
				if (strike >= barrier())
					results_.value = B(-1) - D(-1,-1) + F(-1);
				else
					results_.value = A(-1) - C(-1,-1) + F(-1);
				break;
			}
			break;
		default:
			QL_FAIL("unknown type");
		}
	}


	Real AnalyticPartialTimeBarrierEngine::underlying() const {
		return process_->x0();
	}

	Real AnalyticPartialTimeBarrierEngine::strike() const {
		boost::shared_ptr<PlainVanillaPayoff> payoff =
			boost::dynamic_pointer_cast<PlainVanillaPayoff>(arguments_.payoff);
		QL_REQUIRE(payoff, "non-plain payoff given");
		return payoff->strike();
	}

	Time AnalyticPartialTimeBarrierEngine::residualTime() const {
		return process_->time(arguments_.exercise->lastDate());
	}

	Volatility AnalyticPartialTimeBarrierEngine::volatility() const {
		return process_->blackVolatility()->blackVol(residualTime(), strike());
	}

	Real AnalyticPartialTimeBarrierEngine::stdDeviation() const {
		return volatility() * std::sqrt(residualTime());
	}

	Real AnalyticPartialTimeBarrierEngine::barrier() const {
		return arguments_.barrier;
	}

	Real AnalyticPartialTimeBarrierEngine::rebate() const {
		return arguments_.rebate;
	}

	Rate AnalyticPartialTimeBarrierEngine::riskFreeRate() const {
		return process_->riskFreeRate()->zeroRate(residualTime(), Continuous,
			NoFrequency);
	}

	DiscountFactor AnalyticPartialTimeBarrierEngine::riskFreeDiscount() const {
		return process_->riskFreeRate()->discount(residualTime());
	}

	Rate AnalyticPartialTimeBarrierEngine::dividendYield() const {
		return process_->dividendYield()->zeroRate(residualTime(),
			Continuous, NoFrequency);
	}

	DiscountFactor AnalyticPartialTimeBarrierEngine::dividendDiscount() const {
		return process_->dividendYield()->discount(residualTime());
	}

	Rate AnalyticPartialTimeBarrierEngine::mu() const {
		Volatility vol = volatility();
		return (riskFreeRate() - dividendYield())/(vol * vol) - 0.5;
	}

	Real AnalyticPartialTimeBarrierEngine::muSigma() const {
		return (1 + mu()) * stdDeviation();
	}

	Real AnalyticPartialTimeBarrierEngine::A(Real phi) const {
		Real x1 =
			std::log(underlying()/strike())/stdDeviation() + muSigma();
		Real N1 = f_(phi*x1);
		Real N2 = f_(phi*(x1-stdDeviation()));
		return phi*(underlying() * dividendDiscount() * N1
			- strike() * riskFreeDiscount() * N2);
	}

	Real AnalyticPartialTimeBarrierEngine::B(Real phi) const {
		Real x2 =
			std::log(underlying()/barrier())/stdDeviation() + muSigma();
		Real N1 = f_(phi*x2);
		Real N2 = f_(phi*(x2-stdDeviation()));
		return phi*(underlying() * dividendDiscount() * N1
			- strike() * riskFreeDiscount() * N2);
	}

	Real AnalyticPartialTimeBarrierEngine::C(Real eta, Real phi) const {
		Real HS = barrier()/underlying();
		Real powHS0 = std::pow(HS, 2 * mu());
		Real powHS1 = powHS0 * HS * HS;
		Real y1 = std::log(barrier()*HS/strike())/stdDeviation() + muSigma();
		Real N1 = f_(eta*y1);
		Real N2 = f_(eta*(y1-stdDeviation()));
		return phi*(underlying() * dividendDiscount() * powHS1 * N1
			- strike() * riskFreeDiscount() * powHS0 * N2);
	}

	Real AnalyticPartialTimeBarrierEngine::D(Real eta, Real phi) const {
		Real HS = barrier()/underlying();
		Real powHS0 = std::pow(HS, 2 * mu());
		Real powHS1 = powHS0 * HS * HS;
		Real y2 = std::log(barrier()/underlying())/stdDeviation() + muSigma();
		Real N1 = f_(eta*y2);
		Real N2 = f_(eta*(y2-stdDeviation()));
		return phi*(underlying() * dividendDiscount() * powHS1 * N1
			- strike() * riskFreeDiscount() * powHS0 * N2);
	}

	Real AnalyticPartialTimeBarrierEngine::E(Real eta) const {
		if (rebate() > 0) {
			Real powHS0 = std::pow(barrier()/underlying(), 2 * mu());
			Real x2 =
				std::log(underlying()/barrier())/stdDeviation() + muSigma();
			Real y2 =
				std::log(barrier()/underlying())/stdDeviation() + muSigma();
			Real N1 = f_(eta*(x2 - stdDeviation()));
			Real N2 = f_(eta*(y2 - stdDeviation()));
			return rebate() * riskFreeDiscount() * (N1 - powHS0 * N2);
		} else {
			return 0.0;
		}
	}

	Real AnalyticPartialTimeBarrierEngine::F(Real eta) const {
		if (rebate() > 0) {
			Rate m = mu();
			Volatility vol = volatility();
			Real lambda = std::sqrt(m*m + 2.0*riskFreeRate()/(vol * vol));
			Real HS = barrier()/underlying();
			Real powHSplus = std::pow(HS, m + lambda);
			Real powHSminus = std::pow(HS, m - lambda);

			Real sigmaSqrtT = stdDeviation();
			Real z = std::log(barrier()/underlying())/sigmaSqrtT
				+ lambda * sigmaSqrtT;

			Real N1 = f_(eta * z);
			Real N2 = f_(eta * (z - 2.0 * lambda * sigmaSqrtT));
			return rebate() * (powHSplus * N1 + powHSminus * N2);
		} else {
			return 0.0;
		}
	}
	
	Real AnalyticPartialTimeBarrierEngine::f1() const{
		return std::log(underlying() / strike()) + 2 * std::log(barrier() / underlying()) + (dividendYield() + (std::pow(volatility(),2)/2))*residualTime();
	}

	Real AnalyticPartialTimeBarrierEngine::f2() const{
		return f1() - volatility()*std::sqrt(residualTime());
	}

}

