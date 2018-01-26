#include <iostream>
#include <limits>
#include <vector>
#include <memory>
#include <random>
#include "Profiler.h"
#define POLYMORPH

#if defined POLYMORPH
class Base
{
public:
	Base ( ) = default;
	virtual ~Base ( ) = default;

	virtual void execute ( ) = 0;
};
#endif

template<std::size_t index>
class Derived
#if defined POLYMORPH
	: public Base
#endif
{
public:
	void execute ( )
	{
		result = x * y * z;
	}

private:
	float x { index * index };
	float y { index * index };
	float z { index * index };
	float result;
};

template<std::size_t num_types>
class TypeManager : public TypeManager<num_types - 1>
{
public:
	void AddInstance ( const std::size_t type_index )
	{
		if ( type_index == num_types )
		{
#if defined POLYMORPH
			TypeManager<num_types - 1>::AddInstance ( std::make_unique<Derived<num_types>> ( ) );
#else
			table_.push_back ( { } );
			return;
#endif
		}

		TypeManager<num_types - 1>::AddInstance ( type_index );
	}

#if defined POLYMORPH
	void AddInstance ( std::unique_ptr<Base> new_instance )
	{
		TypeManager<num_types - 1>::AddInstance ( std::move ( new_instance ) );
	}
#endif

	void Update ( )
	{
#if !defined POLYMORPH
		for ( auto it = table_.begin ( ), end = table_.end ( ); it != end; ++it )
		{
			it->execute ( );
		}
#endif

		TypeManager<num_types - 1>::Update ( );
	}

#if !defined POLYMORPH
private:
	std::vector<Derived<num_types>> table_;
#endif
};

template<>
class TypeManager<0>
{
public:
	void AddInstance ( const std::size_t type_index )
	{
#if defined POLYMORPH
		table_.push_back ( std::make_unique<Derived<0>> ( ) );
#else
		table_.push_back ( { } );
		return;
#endif
	}

#if defined POLYMORPH
	void AddInstance ( std::unique_ptr<Base> new_instance )
	{
		table_.push_back ( std::move ( new_instance ) );
	}
#endif

	void Update ( )
	{
		for ( auto it = table_.begin ( ), end = table_.end ( ); it != end; ++it )
		{
#if defined POLYMORPH
			( *it )->execute ( );
#else
			it->execute ( );
#endif
		}
	}


private:
#if defined POLYMORPH
	std::vector<std::unique_ptr<Base>> table_;
#else
	std::vector<Derived<0>> table_;
#endif
};

int main ( )
{
	constexpr auto max_types = size_t { 400 };
	constexpr auto max_objects = size_t { 1000 };
	constexpr auto num_tests = size_t { 1000 };

	Profiler profiler;
	profiler.MakeCurrent ( );

	std::mt19937 rng_engine;
	std::uniform_int_distribution<size_t> rng { 0, max_types };

	TypeManager<max_types> type_manager;

	for ( auto i = size_t { 0 }; i < size_t { max_objects }; ++i )
	{
		const auto decision = rng ( rng_engine );

		type_manager.AddInstance ( decision );
	}

	for ( auto i = size_t { 0 }; i < size_t { num_tests }; ++i )
	{
		profiler.Start ( );
		type_manager.Update ( );
		profiler.End ( );
	}

	auto profile = profiler.Flush ( );

	std::cout << "Lowest time: " << profile.lowest << '\n';
	std::cout << "Highest time: " << profile.highest << '\n';
	std::cout << "Mean: " << profile.mean << '\n';
	std::cout << "Standard deviation: " << profile.standard_deviation << '\n';
	std::cout << "Median: " << profile.median << '\n';

	system ( "pause" );
	return 0;
}