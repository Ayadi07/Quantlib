#include "AnalyticPartialTimeBarrierEngine.h"
#include <ql/exercise.hpp>
#include <ql/quantlib.hpp>

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

		PartialBarrier::Type barrierType = arguments_.barrierType;
		PartialBarrier::Range barrierRange = arguments_.barrierRange;

		switch (payoff->optionType()) {
			//Call Option
		case Option::Call:
			switch (barrierType) {
				//Partial-Time-Start
			case PartialBarrier::Type::DownOut:
				switch (barrierRange)
				{
				case PartialBarrier::Range::Start:
					results_.value = CA(1);
					break;
				case PartialBarrier::Range::EndB1:
					results_.value=CoB1();
					break;
				case PartialBarrier::Range::EndB2:
					results_.value=CoB2(PartialBarrier::Type::DownOut);
					break;
				default:;
				}break;
			case PartialBarrier::Type::DownIn:
				switch (barrierRange)
				{
				case PartialBarrier::Range::Start:
					results_.value = CIA(1);
					break;
				case PartialBarrier::Range::End:
					QL_FAIL("Down and In Partial-Time-End Barrier is not implemented");
					break;
				default:;
				}break;
			case PartialBarrier::Type::UpOut:
				switch (barrierRange)
				{
				case PartialBarrier::Range::Start:
					results_.value = CA(-1);
					break;
				case PartialBarrier::Range::EndB1:
					results_.value=CoB1();
					break;
				case PartialBarrier::Range::EndB2:
					results_.value=CoB2(PartialBarrier::Type::UpOut);
					break;
				default:;
				}break;

			case PartialBarrier::Type::UpIn:
				switch (barrierRange)
				{
				case PartialBarrier::Range::Start:
					results_.value = CIA(-1);
					break;
				case PartialBarrier::Range::End:
					QL_FAIL("Up and In Partial-Time-End Barrier is not implemented");
					break;
				default:;
				}break;
			default:
				QL_FAIL("unknown Partial-Time-Barrier barrierType");
			}break;
		case Option::Put:
			QL_FAIL("Partial-Time Barrier option Put is non-implemented");
			break;
		default:
			QL_FAIL("unknown Option Type");
		}
	}

	Real AnalyticPartialTimeBarrierEngine::CoB2(PartialBarrier::Type barrierType) const{
		Real result=0;
		Real b = riskFreeRate()-dividendYield();
		if(strike()<barrier()){
			switch (barrierType)
			{
			case PartialBarrier::Type::DownOut:
				result = underlying()*std::exp((b-riskFreeRate())*residualTime());
				result *=(M(g1(),e1(),rho())-HS(underlying(),barrier(),2*(mu()+1))*M(g3(),-e3(),-rho()));
				result -=strike()*std::exp(-riskFreeRate()*residualTime())*(M(g2(),e2(),rho())-HS(underlying(),barrier(),2*mu())*M(g4(),-e4(),-rho()));
				return result;break;
			case PartialBarrier::Type::UpOut:
				result = underlying()*std::exp((b-riskFreeRate())*residualTime());
				result *=(M(-g1(),-e1(),rho())-HS(underlying(),barrier(),2*(mu()+1))*M(-g3(),e3(),-rho()));
				result -=strike()*std::exp(-riskFreeRate()*residualTime())*(M(-g2(),-e2(),rho())-HS(underlying(),barrier(),2*mu())*M(-g4(),e4(),-rho()));
				result -=underlying()*std::exp((b-riskFreeRate())*residualTime())*(M(-d1(),-e1(),rho())-HS(underlying(),barrier(),2*(mu()+1))*M(e3(),-f1(),-rho()));
				result +=strike()*std::exp(-riskFreeRate()*residualTime())*(M(-d2(),-e2(),rho())-HS(underlying(),barrier(),2*mu())*M(e4(),-f2(),-rho()));
				return result;break;
			default:
				QL_FAIL("non-implemented Partial-Time-End Barrier barrierType");
				return 0;
				break;
			}
		}else
			QL_FAIL("case: strike()>barrier(): is not implemented for OutEnd B2 type");
		return 0;
	}

	Real AnalyticPartialTimeBarrierEngine::CoB1() const{
		Real result = 0.0;
		Real strike2 = strike();
		Real bar = barrier();
		Real under = underlying();
		Real resi = residualTime();
		Real rateI = riskFreeRate();
		Real divi = dividendYield();
		Real T1 = coverEventTime();
		Real b = riskFreeRate()-dividendYield();
		if (strike()>barrier())
		{
			result = underlying()*std::exp((b-riskFreeRate())*residualTime());
			result *=(M(d1(),e1(),rho())-HS(underlying(),barrier(),2*(mu()+1))*M(f1(),-e3(),-rho()));
			result -=(strike()*std::exp(-riskFreeRate()*residualTime()))*(M(d2(),e2(),rho())-HS(underlying(),barrier(),2*mu())*M(f2(),-e4(),-rho()));
			return result;
		}else{
			Real S1 = underlying()*std::exp((b-riskFreeRate())*residualTime());
			Real X1 = (strike()*std::exp(-riskFreeRate()*residualTime()));
			Real HS1 = HS(underlying(),barrier(),2*(mu()+1));
			Real HS2 = HS(underlying(), barrier(), 2 * mu());
			result = S1;
			result *=(M(-g1(),-e1(),rho())-HS1*M(-g3(),e3(),-rho()));
			result -=X1*(M(-g2(), -e2(), rho()) - HS2*M(-g4(), e4(), -rho()));
			result -=S1*(M(-d1(), -e1(), rho()) - HS1*M(-f1(), e3(), -rho()));
			result +=X1*(M(-d2(), -e2(), rho()) - HS2*M(-f2(), e4(), -rho()));
			result +=S1*(M(g1(), e1(), rho()) - HS1*M(g3(), -e3(), -rho()));
			result -=X1*(M(g2(), e2(), rho()) - HS2*M(g4(), -e4(), -rho()));
			return result;
		}
	}

	//arg n : -1 Up-and-In Call
	//arg n :  1 Down-and-In Call 
	Real AnalyticPartialTimeBarrierEngine::CIA(Integer eta) const{
		//Calcul Vannilla Call Option
		boost::shared_ptr<EuropeanExercise> exercise =
			boost::dynamic_pointer_cast<EuropeanExercise>(arguments_.exercise);

		boost::shared_ptr<PlainVanillaPayoff> payoff =
			boost::dynamic_pointer_cast<PlainVanillaPayoff>(arguments_.payoff);

		VanillaOption europeanOption(payoff, exercise);

		europeanOption.setPricingEngine(boost::shared_ptr<PricingEngine>(
			new AnalyticEuropeanEngine(process_)));

		//Calcul result
		return europeanOption.NPV() - CA(eta);
	}

	Real AnalyticPartialTimeBarrierEngine::CA(Integer eta) const{
		Real b = riskFreeDiscount()-dividendYield();
		//Partial-Time-Start- OUT  Call Option calculation 
		if(std::abs(eta)==1){
			Real result;
			result = underlying()*std::exp((b-riskFreeRate())*residualTime());
			result *=(M(d1(),eta*e1(),eta*rho())-HS(underlying(),barrier(),2*(mu()+1))*M(f1(),eta*e3(),eta*rho()));
			result -=(strike()*std::exp(-riskFreeRate()*residualTime())*(M(d2(),eta*e2(),eta*rho())-HS(underlying(),barrier(),2*mu())*M(f2(),eta*e4(),eta*rho())));
			return result;
		}
		else{
			QL_FAIL("Error in AnalyticPartialBarrierEngine: CA wrong n given ( must be 1 or -1)");
			return 0;
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

	Time AnalyticPartialTimeBarrierEngine::coverEventTime() const{
		return process_->time(arguments_.coverEventDate);
	}

	Volatility AnalyticPartialTimeBarrierEngine::volatility(int i) const {
		if(i == 1){
			return process_->blackVolatility()->blackVol(coverEventTime(), strike());
		}else{
			return process_->blackVolatility()->blackVol(residualTime(), strike());
		}
	}

	Real AnalyticPartialTimeBarrierEngine::stdDeviation() const {
		return volatility(2) * std::sqrt(residualTime());
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


	Real AnalyticPartialTimeBarrierEngine::f1() const{
		Real _underlying = underlying();
		Real _volatility = volatility(2);
		Real _residualTime = residualTime();
		return (std::log(_underlying / strike()) + 2 * std::log(barrier() / _underlying) + ((riskFreeRate()-dividendYield()) + (std::pow(_volatility, 2) / 2))*_residualTime) / (_volatility*std::sqrt(_residualTime));
	}

	Real AnalyticPartialTimeBarrierEngine::f2() const{
		return f1() - volatility(2)*std::sqrt(residualTime());
	}

	Real AnalyticPartialTimeBarrierEngine::M(Real a,Real b,Real rho) const{
		BivariateCumulativeNormalDistributionDr78 CmlNormDist(rho);
		return CmlNormDist(a,b);
	}

	Real AnalyticPartialTimeBarrierEngine::rho() const{
		return std::sqrt(coverEventTime()/residualTime());
	}

	Rate AnalyticPartialTimeBarrierEngine::mu() const{
		Volatility vol = volatility(1);
		return ((riskFreeRate() - dividendYield()) - (vol * vol) / 2) / (vol * vol);
	}

	Real AnalyticPartialTimeBarrierEngine::d1()const{
		Volatility vol = volatility(2);
		Real b= riskFreeRate()-dividendYield();
		Time T2=residualTime();
		return (std::log(underlying()/strike())+(b+vol*vol/2)*T2)/(std::sqrt(T2)*vol);
	}

	Real AnalyticPartialTimeBarrierEngine::d2()const{
		Volatility vol = volatility(2);
		Time T2=residualTime();
		return d1() - vol*std::sqrt(T2);
	}

	Real AnalyticPartialTimeBarrierEngine::e1()const{
		Volatility vol = volatility(1);
		Real b= riskFreeRate()-dividendYield();
		Time T1=coverEventTime();
		return (std::log(underlying()/barrier())+(b+vol*vol/2)*T1)/(std::sqrt(T1)*vol);
	}

	Real AnalyticPartialTimeBarrierEngine::e2()const{
		Volatility vol = volatility(1);
		Time T1=coverEventTime();
		return e1() - vol*std::sqrt(T1);
	}

	Real AnalyticPartialTimeBarrierEngine::e3()const{
		Time T1=coverEventTime();
		Real vol=volatility(1);
		return e1()+(2*std::log(barrier()/underlying()) /(vol*std::sqrt(T1)));
	}

	Real AnalyticPartialTimeBarrierEngine::e4()const{
		return e3()-volatility(1)*std::sqrt(coverEventTime());
	}

	Real AnalyticPartialTimeBarrierEngine::g1()const{
		Volatility vol = volatility(2);
		Real b= riskFreeRate()-dividendYield();
		Time T2=residualTime();
		return (std::log(underlying()/barrier())+(b+vol*vol/2)*T2)/(std::sqrt(T2)*vol);
	}

	Real AnalyticPartialTimeBarrierEngine::g2()const{
		Volatility vol = volatility(2);
		Time T2=residualTime();
		return g1() - vol*std::sqrt(T2);
	}

	Real AnalyticPartialTimeBarrierEngine::g3()const{
		Time T2=residualTime();
		Real vol=volatility(2);
		return g1()+(2*std::log(barrier()/underlying()) /(vol*std::sqrt(T2)));
	}

	Real AnalyticPartialTimeBarrierEngine::g4()const{
		Time T2=residualTime();
		Real vol=volatility(2);
		return g3()-vol*std::sqrt(T2);
	}
	Real AnalyticPartialTimeBarrierEngine::HS(Real S, Real H, Real power) const{
		return std::pow((H/S),power);
	}

}
