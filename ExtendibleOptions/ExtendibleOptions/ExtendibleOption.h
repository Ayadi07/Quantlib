#pragma once

#include <ql/instruments/payoffs.hpp>
#include <ql/instruments/oneassetoption.hpp>
#include <ql/errors.hpp>

namespace QuantLib{
	struct ExtendibleOptionType
	{
		enum Type { W, H};
	};
	class GeneralizedBlackScholesProcess;
	class ExtendibleOption:public OneAssetOption
	{
	public:
		class arguments;
		class engine;	
		ExtendibleOption(
			Option::Type type,
			ExtendibleOptionType::Type writerHolder,
			Real premium,
			Date secondExpiryDate,
			Real secondStrike,
			const boost::shared_ptr<StrikedTypePayoff>& payoff,
			const boost::shared_ptr<Exercise>& exercise);
		~ExtendibleOption(void);
		void setupArguments(PricingEngine::arguments*) const;
	protected:
		ExtendibleOptionType::Type writerHolder_;
		Real premium_;
		Date secondExpiryDate_;
		Real secondStrike_;
	};
	class ExtendibleOption::arguments : public OneAssetOption::arguments {
	public:
		arguments();
		ExtendibleOptionType::Type writerHolder;
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