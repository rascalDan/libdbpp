#include "testCore.h"
#include <selectcommand.h>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/test/test_tools.hpp>
#include <compileTimeFormatter.h>
#include <fileUtils.h>

namespace DB {

TestCore::TestCore() :
	testString("Some C String"),
	testDateTime(boost::posix_time::from_time_t(1430530593)),
	testInterval(boost::posix_time::time_duration(1, 2, 3)),
	testBlobData([](){
		AdHoc::FileUtils::MemMap f("/proc/self/exe");
		return std::vector<unsigned char>(f.sv<unsigned char>().begin(), f.sv<unsigned char>().end());
	}()),
	testBlob(testBlobData)
{
}

template<typename T>
class Assert : public DB::HandleField {
	public:
		explicit Assert(const T & e) : expected(e) { }

		void floatingpoint(double v) override { (*this)(v); }
		void integer(int64_t v) override { (*this)(v); }
		void boolean(bool v) override { (*this)(v); }
		void string(const std::string_view v) override { (*this)(v); }
		void timestamp(const boost::posix_time::ptime & v) override { (*this)(v); }
		void interval(const boost::posix_time::time_duration & v) override { (*this)(v); }
		void blob(const Blob & v) override { (*this)(v); }
		void null() override { }

		template <typename D>
		void operator()(const D & v) {
			if constexpr (std::is_convertible<D, T>::value) {
				BOOST_REQUIRE_EQUAL(expected, v);
			}
			else {
				// NOLINTNEXTLINE(hicpp-vararg)
				BOOST_ERROR("Unexpected column type " << typeid(D).name());
			}
		}

	private:
		const T & expected;
};

template<typename T>
void
TestCore::assertScalarValueHelper(DB::SelectCommand & sel, const T & t) const
{
	while (sel.fetch()) {
		assertColumnValueHelper(sel, 0, t);
	}
}

template<typename T>
void
TestCore::assertColumnValueHelper(DB::SelectCommand & sel, unsigned int col, const T & t) const
{
	Assert<T> a(t);
	sel[col].apply(a);
}

AdHocFormatter(BlobDbg, "Blob[length=%?, addr=%?]");
std::ostream &
operator<<(std::ostream & s, const DB::Blob b)
{
	BlobDbg::write(s, b.len, b.data);
	return s;
}

template void TestCore::assertScalarValueHelper<bool>(SelectCommand &, const bool &) const;
template void TestCore::assertScalarValueHelper<int64_t>(SelectCommand &, const int64_t &) const;
template void TestCore::assertScalarValueHelper<int>(SelectCommand &, const int &) const;
template void TestCore::assertScalarValueHelper<double>(SelectCommand &, const double &) const;
template void TestCore::assertScalarValueHelper<std::string_view>(SelectCommand &, const std::string_view &) const;
template void TestCore::assertScalarValueHelper<boost::posix_time::ptime>(SelectCommand &, const boost::posix_time::ptime &) const;
template void TestCore::assertScalarValueHelper<boost::posix_time::time_duration>(SelectCommand &, const boost::posix_time::time_duration &) const;
template void TestCore::assertScalarValueHelper<DB::Blob>(SelectCommand &, const DB::Blob &) const;

template void TestCore::assertColumnValueHelper<bool>(SelectCommand &, unsigned int, const bool &) const;
template void TestCore::assertColumnValueHelper<int>(SelectCommand &, unsigned int, const int &) const;
template void TestCore::assertColumnValueHelper<int64_t>(SelectCommand &, unsigned int, const int64_t &) const;
template void TestCore::assertColumnValueHelper<double>(SelectCommand &, unsigned int, const double &) const;
template void TestCore::assertColumnValueHelper<std::string_view>(SelectCommand &, unsigned int, const std::string_view &) const;
template void TestCore::assertColumnValueHelper<boost::posix_time::ptime>(SelectCommand &, unsigned int, const boost::posix_time::ptime &) const;
template void TestCore::assertColumnValueHelper<boost::posix_time::time_duration>(SelectCommand &, unsigned int, const boost::posix_time::time_duration &) const;
template void TestCore::assertColumnValueHelper<DB::Blob>(SelectCommand &, unsigned int, const DB::Blob &) const;

}

