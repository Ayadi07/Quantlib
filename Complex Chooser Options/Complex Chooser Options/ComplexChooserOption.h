#pragma once
#include <ql/instruments/payoffs.hpp>
#include <ql/instruments/oneassetoption.hpp>


namespace QuantLib{

	class GeneralizedBlackScholesProcess;
	class ComplexChooserOption :
		public OneAssetOption
	{
	public:
		class arguments;
		class engine;		
		ComplexChooserOption(
			Date choosingDate,
			Real strikeCall,
			Real strikePut,
			const boost::shared_ptr<Exercise>& exerciseCall,
			const boost::shared_ptr<Exercise>& exercisePut);
		void setupArguments(PricingEngine::arguments*) const;

	protected:
		Date choosingDate_;
		Real strikeCall_;
		Real strikePut_;
		const boost::shared_ptr<Exercise>& exerciseCall_;
		const boost::shared_ptr<Exercise>& exercisePut_;
		~ComplexChooserOption(void);
	};

	class ComplexChooserOption::arguments
		: public OneAssetOption::arguments {
	public:
		//arguments() : choosingDate(Null<Date>()) {};
		arguments();
		void validate() const;
		Date choosingDate;
		Real strikeCall;
		Real strikePut;
		boost::shared_ptr<Exercise>& exerciseCall;
		boost::shared_ptr<Exercise>& exercisePut;
	};
	class ComplexChooserOption::engine
		: public GenericEngine<ComplexChooserOption::arguments,
		ComplexChooserOption::results> {};
}
