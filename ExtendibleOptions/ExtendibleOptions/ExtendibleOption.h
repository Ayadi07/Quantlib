#pragma once

#include <ql/instruments/payoffs.hpp>
#include <ql/instruments/oneassetoption.hpp>
#include <ql/errors.hpp>

namespace QuantLib{
	class GeneralizedBlackScholesProcess;
	class ExtendibleOption:public OneAssetOption
	{
	public:
		enum Type { Writer, Holder };
		class arguments;
		class engine;	
		ExtendibleOption(
			Option::Type type,
			ExtendibleOption::Type writerHolder,
			Real premium,
			Date secondExpiryDate,
			Real secondStrike,
			const boost::shared_ptr<StrikedTypePayoff>& payoff,
			const boost::shared_ptr<Exercise>& exercise);
		~ExtendibleOption(void);
		void setupArguments(PricingEngine::arguments*) const;
	protected:
		ExtendibleOption::Type writerHolder_;
		Real premium_;
		Date secondExpiryDate_;
		Real secondStrike_;
	};
	class ExtendibleOption::arguments : public OneAssetOption::arguments {
	public:
		//arguments();
		ExtendibleOption::Type writerHolder;
		Real premium;
		Date secondExpiryDate;
		Real secondStrike;

		void validate() const;
	};
	class ExtendibleOption::engine
		: public GenericEngine<ExtendibleOption::arguments,
		ExtendibleOption::results> {
	};
}