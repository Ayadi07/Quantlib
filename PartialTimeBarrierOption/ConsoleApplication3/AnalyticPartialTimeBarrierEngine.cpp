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

		PartialBarrier::Type barrierType = arguments_.barrierType;

		switch (payoff->optionType()) {
			//Call Option
		case Option::Call:
			switch (barrierType) {
				//Partial-Time-Start
			case PartialBarrier::DownOutStart:
				results_.value = CA(strike,1);
				break;
			case PartialBarrier::UpOutStart:
				results_.value = CA(strike,-1);
				break;
			case PartialBarrier::DownInStart:
				break;
			case PartialBarrier::UpInStart:
				break;
				//Partial-Time-Start //end
				//Partial-Time-End
			case PartialBarrier::UpOutEnd:
				break;
			case PartialBarrier::DownOutEnd:
				break;
			case PartialBarrier::UpInEnd:
				break;
			case PartialBarrier::DownInEnd:
				break;
				//Partial-Time-End //end
			default:
				QL_FAIL("unknown Partial-Time-Barrier barrierType");
			}
			case Option::Put:
				QL_FAIL("Implemented option");
				break;
			default:
				QL_FAIL("unknown Type");
			}
		}

		Real AnalyticPartialTimeBarrierEngine::CA(Real strike,Integer n) const
		{
			//Partial-Time-Start- OUT  Call Option calculation 
			if(std::abs(n)==1){
				Real result;
				result = underlying()*std::expl(dividendYield()-InterestRate()*residualTime());
				result *=(M(d1(),e1(),ro(),n)-HS(underlying(),barrier(),2*(mu()+1))*M(f1(),e3(),ro(),n));
				result -=(strike*std::expl(-InterestRate()*residualTime())*(M(d2(),e2(),ro(),n)-HS(underlying(),barrier(),mu())*M(f2(),e4(),ro(),n)));
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
			return std::log(underlying() / strike()) + 2 * std::log(barrier() / underlying()) + (dividendYield() + (std::pow(volatility(),2)/2))*residualTime();
		}

		Real AnalyticPartialTimeBarrierEngine::f2() const{
			return f1() - volatility()*std::sqrt(residualTime());
		}
		Real AnalyticPartialTimeBarrierEngine::M(Real a,Real b,Real ro,Integer n) const 
		{

			CumulativeNormalDistribution CmlNormDist(a,n*b);
			return CmlNormDist(n*ro);
		}
		Real AnalyticPartialTimeBarrierEngine::ro() const
		{
			SQRT(coverEventDate()/residualTime());
		}
		Rate AnalyticPartialTimeBarrierEngine::mu() const {
			Volatility vol = volatility();
			return (dividendYield()/(vol * vol)) - 0.5;
		}

		Real AnalyticPartialTimeBarrierEngine::d1()const
		{
			Volatility vol = volatility();
			Real b=dividendYield();
			Time T2=residualTime();\
				return (LOG(underlying()/strike())+(b+vol*vol/2)*SQRT(T2))/vol;
		}
		Real AnalyticPartialTimeBarrierEngine::d2()const
		{
			Volatility vol = volatility();
			Real b=dividendYield();
			Time T2=residualTime();
			return (LOG(underlying()/strike())+(b+vol*vol/2)*SQRT(T2))/vol - vol*SQRT(T2);
		}

		Real AnalyticPartialTimeBarrierEngine::e1()const
		{
			Volatility vol = volatility();
			Real b=dividendYield();
			Time T1=coverEventDate();


			return (LOG(underlying()/barrier())+(b+vol*vol/2)*SQRT(T1))/vol;


		}
		Real AnalyticPartialTimeBarrierEngine::e2()const
		{
			Volatility vol = volatility();
			Real b=dividendYield();
			Time T1=coverEventDate();

			return (LOG(underlying()/barrier())+(b+vol*vol/2)*SQRT(T1))/vol - vol*SQRT(T1);
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
		Real LOG(Real r)
		{
			return std::log(r);
		}
		Real SQRT(Real r){
			return std::sqrt(r);
		}
		Real HS(Real S, Real H, Real power)
		{
			return std::powl((H/S),power);
		}

	}

