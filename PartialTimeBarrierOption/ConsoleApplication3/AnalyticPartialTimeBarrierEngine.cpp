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
		//QL_REQUIRE(!triggered(spot), "barrier touched");

		PartialBarrier::Type barrierType = arguments_.barrierType;

		switch (payoff->optionType()) {
			//Call Option
		case Option::Call:
			switch (barrierType) {
				//Partial-Time-Start
			case PartialBarrier::DownOutStart:
				results_.value = CA(1);
				break;
			case PartialBarrier::UpOutStart:
				results_.value = CA(-1);
				break;
			case PartialBarrier::DownInStart:
				results_.value = CIA(1);
				break;
			case PartialBarrier::UpInStart:
				results_.value = CIA(-1);
				break;
				//Partial-Time-Start //end
				//Partial-Time-End
			case PartialBarrier::UpOutEndB1:
				results_.value=CoB1();
				break;
			case PartialBarrier::UpOutEndB2:
				results_.value=CoB2(PartialBarrier::UpOutEndB2);
				break;
			case PartialBarrier::DownOutEndB1:
				results_.value=CoB1();
				break;
			case PartialBarrier::DownOutEndB2:
				results_.value=CoB2(PartialBarrier::DownOutEndB2);
				break;
			case PartialBarrier::UpInEnd:
				QL_FAIL("Up and In Partial-Time-End Barrier is not implemented");
				break;
			case PartialBarrier::DownInEnd:
				QL_FAIL("Down and In Partial-Time-End Barrier is not implemented");
				break;
				//Partial-Time-End //end
			default:
				QL_FAIL("unknown Partial-Time-Barrier barrierType");
			}
			break;
		case Option::Put:
			QL_FAIL("Partial-Time Barrier option Put is non-implemented");
			break;
		default:
			QL_FAIL("unknown Type");
		}
	}
	Real AnalyticPartialTimeBarrierEngine::CoB2(PartialBarrier::Type barrierType) const
	{
		Real result;
		if(strike()<barrier()){
			switch (barrierType)
			{
			case PartialBarrier::UpOutEndB2:
				result = underlying()*std::exp((dividendYield()-InterestRate())*residualTime());
				result *=(M(g1(),e1(),rho(),1)-HS(underlying(),barrier(),2*(mu()+1))*M(g3(),-e3(),-rho(),1));
				result -=strike()*std::exp(-InterestRate()*residualTime())*(M(g2(),e2(),rho(),1)-HS(underlying(),barrier(),2*mu())*M(g4(),-e4(),-rho(),1));
				return result;break;
			case PartialBarrier::DownOutEndB2:
				result = underlying()*std::exp((dividendYield()-InterestRate())*residualTime());
				result *=(M(-g1(),-e1(),rho(),1)-HS(underlying(),barrier(),2*(mu()+1))*M(-g3(),e3(),-rho(),1));
				result -=strike()*std::exp(-InterestRate()*residualTime())*(M(-g2(),-e2(),rho(),1)-HS(underlying(),barrier(),2*mu())*M(-g4(),e4(),-rho(),1));
				result -=underlying()*std::exp((dividendYield()-InterestRate())*residualTime())*(M(-d1(),-e1(),rho(),1)-HS(underlying(),barrier(),2*(mu()+1))*M(e3(),-f1(),-rho(),1));
				result +=strike()*std::exp(-InterestRate()*residualTime())*(M(-d2(),-e2(),rho(),1)-HS(underlying(),barrier(),2*mu())*M(e4(),-f2(),-rho(),1));
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
	Real AnalyticPartialTimeBarrierEngine::CoB1() const
	{

		Real result;
		Real strike2 = strike();
		Real bar = barrier();
		Real under = underlying();
		Real resi = residualTime();
		Real rateI = riskFreeRate();
		Real divi = dividendYield();
		Real T1 = coverEventDate();

		if (strike()>barrier())
		{

			result = underlying()*std::exp((dividendYield()-riskFreeRate())*residualTime());
			result *=(M(d1(),e1(),rho(),1)-HS(underlying(),barrier(),2*(mu()+1))*M(f1(),-e3(),-rho(),-1));
			result -=(strike()*std::exp(-riskFreeRate()*residualTime()))*(M(d2(),e2(),rho(),1)-HS(underlying(),barrier(),2*mu())*M(f2(),-e4(),-rho(),-1));
			return result;
		}else{
			result = underlying()*std::exp((dividendYield() - riskFreeRate())*residualTime());
			result *=(M(-g1(),-e1(),rho(),1)-HS(underlying(),barrier(),2*(mu()+1))*M(-g3(),e3(),-rho(),1));
			result -= (strike()*std::exp(-riskFreeRate()*residualTime())*(M(-g2(), -e2(), rho(), 1) - HS(underlying(), barrier(), 2 * mu())*M(-g4(), e4(), -rho(), 1)));
			result -= underlying()*std::exp((dividendYield() - riskFreeRate())*residualTime())*(M(-d1(), -e1(), rho(), 1) - HS(underlying(), barrier(), 2 * (mu() + 1))*M(-f1(), e3(), -rho(), 1));
			result += (strike()*std::exp(-riskFreeRate()*residualTime())*(M(-d2(), -e2(), rho(), 1) - HS(underlying(), barrier(), 2 * mu())*M(-f2(), e4(), -rho(), 1)));
			result += underlying()*std::exp((dividendYield() - riskFreeRate())*residualTime())*(M(g1(), e1(), rho(), 1) - HS(underlying(), barrier(), 2 * (mu() + 1))*M(g3(), -e3(), -rho(), 1));
			result -= (strike()*std::exp(-riskFreeRate()*residualTime())*(M(g2(), e2(), rho(), 1) - HS(underlying(), barrier(), 2 * mu())*M(g4(), -e4(), -rho(), 1)));
			return result;
		}
	}

	//arg n : -1 Up-and-In Call
	//arg n :  1 Down-and-In Call 
	Real AnalyticPartialTimeBarrierEngine::CIA(Integer n) const
	{
		//Calcul Vannilla Call Option
		boost::shared_ptr<EuropeanExercise> exercise =
			boost::dynamic_pointer_cast<EuropeanExercise>(arguments_.exercise);

		boost::shared_ptr<PlainVanillaPayoff> payoff =
			boost::dynamic_pointer_cast<PlainVanillaPayoff>(arguments_.payoff);

		VanillaOption europeanOption(payoff, exercise);

		europeanOption.setPricingEngine(boost::shared_ptr<PricingEngine>(
			new AnalyticEuropeanEngine(process_)));

		//Calcul result
		return europeanOption.NPV() - CA(n);
	}

	Real AnalyticPartialTimeBarrierEngine::CA(Integer n) const
	{
		//Partial-Time-Start- OUT  Call Option calculation 
		if(std::abs(n)==1){
			Real result;
			result = underlying()*std::exp((dividendYield()-InterestRate())*residualTime());
			result *=(M(d1(),e1(),rho(),n)-HS(underlying(),barrier(),2*(mu()+1))*M(f1(),e3(),rho(),n));
			result -=(strike()*std::exp(-InterestRate()*residualTime())*(M(d2(),e2(),rho(),n)-HS(underlying(),barrier(),2*mu())*M(f2(),e4(),rho(),n)));
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
	Time AnalyticPartialTimeBarrierEngine::coverEventDate() const
	{
		return process_->time(arguments_.coverEventDate);
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


	Real AnalyticPartialTimeBarrierEngine::f1() const{
		return (std::log(underlying() / strike()) + 2 * std::log(barrier() / underlying()) + (dividendYield() + (std::pow(volatility(), 2) / 2))*residualTime()) / (volatility()*SQRT(residualTime()));
	}

	Real AnalyticPartialTimeBarrierEngine::f2() const{
		return f1() - volatility()*std::sqrt(residualTime());
	}
	Real AnalyticPartialTimeBarrierEngine::M(Real a,Real b,Real rho,Integer n) const 
	{

		BivariateCumulativeNormalDistributionDr78 CmlNormDist(rho);
		return CmlNormDist(a,b);
	}
	Real AnalyticPartialTimeBarrierEngine::rho() const
	{
		return std::sqrt(coverEventDate()/residualTime());
	}
	Rate AnalyticPartialTimeBarrierEngine::mu() const {
		Volatility vol = volatility();
		return (dividendYield() - (vol * vol) / 2) / (vol * vol);
	}

	Real AnalyticPartialTimeBarrierEngine::d1()const
	{
		Volatility vol = volatility();
		Real b=dividendYield();
		Time T2=residualTime();
		return (LOG(underlying()/strike())+(b+vol*vol/2)*T2)/(SQRT(T2)*vol);
	}
	Real AnalyticPartialTimeBarrierEngine::d2()const
	{
		Volatility vol = volatility();
		Time T2=residualTime();
		return d1() - vol*SQRT(T2);
	}

	Real AnalyticPartialTimeBarrierEngine::e1()const
	{
		Volatility vol = volatility();
		Real b=dividendYield();
		Time T1=coverEventDate();


		return (LOG(underlying()/barrier())+(b+vol*vol/2)*T1)/(SQRT(T1)*vol);


	}
	Real AnalyticPartialTimeBarrierEngine::e2()const
	{
		Volatility vol = volatility();
		Real b=dividendYield();
		Time T1=coverEventDate();

		return e1() - vol*SQRT(T1);
	}

	Real AnalyticPartialTimeBarrierEngine::e3()const
	{
		Time T1=coverEventDate();
		Real vol=volatility();
		return e1()+(2*LOG(barrier()/underlying()) /(vol*SQRT(T1)));
	}

	Real AnalyticPartialTimeBarrierEngine::e4()const
	{
		return e3()-volatility()*SQRT(coverEventDate());
	}

	Real AnalyticPartialTimeBarrierEngine::g1()const
	{
		Volatility vol = volatility();
		Real b=dividendYield();
		Time T2=residualTime();
		return (LOG(underlying()/barrier())+(b+vol*vol/2)*T2)/(SQRT(T2)*vol);
	}
	Real AnalyticPartialTimeBarrierEngine::g2()const
	{
		Volatility vol = volatility();
		Time T2=residualTime();
		return g1() - vol*SQRT(T2);
	}
	Real AnalyticPartialTimeBarrierEngine::g3()const{
		Time T2=residualTime();
		Real vol=volatility();
		return g1()+(2*LOG(barrier()/underlying()) /(vol*SQRT(T2)));
	}
	Real AnalyticPartialTimeBarrierEngine::g4()const{
		Time T2=residualTime();
		Real vol=volatility();
		return g3()-vol*SQRT(T2);
	}
	Real AnalyticPartialTimeBarrierEngine::LOG(Real r) const
	{
		return std::log(r);
	}
	Real AnalyticPartialTimeBarrierEngine::SQRT(Real r) const{
		return std::sqrt(r);
	}
	Real AnalyticPartialTimeBarrierEngine::HS(Real S, Real H, Real power) const
	{
		return std::pow((H/S),power);
	}

}
