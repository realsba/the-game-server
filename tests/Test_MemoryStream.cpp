//
// Created by sba <bohdan.sadovyak@gmail.com> on 10.12.23.
//

#include <catch2/catch_test_macros.hpp>

#include <sstream>

#include "../src/MemoryStream.hpp"

using Direction = MemoryStream::Direction;

TEST_CASE("MemoryStream: Constructor", "[MemoryStream]")
{
  MemoryStream ms;
  CHECK(ms.blockSize() == 256);
  CHECK(ms.capacity() == 0);
  CHECK(ms.data() == nullptr);
  CHECK(ms.length() == 0);
  CHECK(ms.tellG() == 0);
  CHECK(ms.tellP() == 0);
  CHECK(ms.availableForRead() == 0);
  CHECK(ms.availableForWrite() == 0);
}

TEST_CASE("MemoryStream: Copy Constructor", "[MemoryStream]")
{
  MemoryStream ms;
  MemoryStream ms2(ms);

  CHECK(ms2.capacity() == 0);
  CHECK(ms2.data() == nullptr);
  CHECK(ms2.length() == 0);
  CHECK(ms2.tellG() == 0);
  CHECK(ms2.tellP() == 0);

  ms.writeUInt32(0x12345678);
  ms.writeString("LOL2023");
  ms.seekG(7, Direction::Begin);
  ms.seekP(8, Direction::Begin);

  auto* data = ms.data();
  auto pos_g = ms.tellG();
  auto pos_p = ms.tellP();
  auto cap = ms.capacity();
  auto length = ms.length();

  MemoryStream ms3(ms);
  CHECK(ms.capacity() == cap);
  CHECK(ms.data() == data);
  CHECK(ms.length() == length);
  CHECK(ms.tellG() == pos_g);
  CHECK(ms.tellP() == pos_p);

  CHECK(ms3.capacity() == cap);
  CHECK(ms3.data() != nullptr);
  CHECK(ms3.data() != data);
  CHECK(ms3.length() == length);
  CHECK(ms3.tellG() == pos_g);
  CHECK(ms3.tellP() == pos_p);

  ms3.seekG(0, Direction::Begin);
  CHECK(ms3.readUInt32() == 0x12345678);
  CHECK(ms3.readString() == "LOL2023");
}

TEST_CASE("MemoryStream: assignment", "[MemoryStream]")
{
  MemoryStream ms;

  MemoryStream ms2(1000);
  ms2.writeString("LOL2023");
  ms2.seekG(0, Direction::End);

  ms2 = ms;

  CHECK(ms2.blockSize() == 256);
  CHECK(ms2.length() == 0);
  CHECK(ms2.tellG() == 0);
  CHECK(ms2.tellP() == 0);

  ms.writeInt32(0x12345678);
  ms.writeString("LOL2023");
  ms.seekG(7, Direction::Begin);
  ms.seekP(8, Direction::Begin);

  auto* data = ms.data();
  auto cap = ms.capacity();
  auto length = ms.length();
  auto pos_g = ms.tellG();
  auto pos_p = ms.tellP();

  MemoryStream ms3;
  ms3.writeDouble(3.14);
  ms3 = ms;

  CHECK(ms.capacity() == cap);
  CHECK(ms.data() == data);
  CHECK(ms.length() == length);
  CHECK(ms.tellG() == pos_g);
  CHECK(ms.tellP() == pos_p);

  CHECK(ms3.capacity() == cap);
  CHECK(ms3.data() != nullptr);
  CHECK(ms3.data() != data);
  CHECK(ms3.length() == length);
  CHECK(ms3.tellG() == pos_g);
  CHECK(ms3.tellP() == pos_p);

  ms3.seekG(0, Direction::Begin);
  CHECK(ms3.readInt32() == 0x12345678);
  CHECK(ms3.readString() == "LOL2023");
}

TEST_CASE("MemoryStream: clear", "[MemoryStream]")
{
  MemoryStream ms;
  ms.writeInt32(42);

  CHECK(ms.data() != nullptr);
  CHECK(ms.length() != 0);
  CHECK(ms.tellG() == 0);
  CHECK(ms.tellP() != 0);

  ms.clear();
  CHECK(ms.data() != nullptr);
  CHECK(ms.length() == 0);
  CHECK(ms.tellG() == 0);
  CHECK(ms.tellP() == 0);
}

TEST_CASE("MemoryStream: compress", "[MemoryStream]")
{
  MemoryStream ms;
  ms.writeInt32(42);
  ms.writeInt32(42);

  ms.seekG(1, Direction::Begin);
  ms.seekP(2, Direction::Begin);
  ms.compress();
  CHECK(ms.data() != nullptr);
  CHECK(ms.length() == 7);

  ms.seekP(3, Direction::Begin);
  ms.seekG(4, Direction::Begin);
  ms.compress();
  CHECK(ms.length() == 4);

  ms.seekP(0, Direction::End);
  ms.seekG(0, Direction::End);
  ms.compress();
  CHECK(ms.length() == 0);

}

TEST_CASE("MemoryStream: file operations", "[MemoryStream]")
{
// TODO: implement test case
//  using namespace  boost::filesystem3;
//
//  path path = temp_directory_path() / unique_path();
//
//  MemoryStream ms;
//  ms.WriteUInt32(42);
//  ms.writeString("42");
//  ms.SaveToFile(path.string());
//  MemoryStream ms2;
//  ms2.WriteDouble(2.71);
//  ms2.LoadFromFile(path.string());
//
//  remove(path);
//
//  CHECK(ms2.length() == ms.length());
//  CHECK(ms2.tellG() == 0);
//  CHECK(ms2.tellP() == ms2.length());
//  CHECK(ms2.readUInt32() == 42);
//  CHECK(ms2.readString() == "42");
}

TEST_CASE("MemoryStream: peek", "[MemoryStream]")
{
  MemoryStream ms;
  ms.writeUInt32(0xAABBCCDD);
  ms.writeUInt32(0x12345678);

  CHECK(ms.peekUInt32() == 0xAABBCCDD);
  CHECK(ms.peekUInt32() == 0xAABBCCDD);
  CHECK(ms.length() == 8);
  CHECK(ms.tellG() == 0);
  CHECK(ms.tellP() == ms.length());

  ms.seekG(0, Direction::End);
  CHECK_THROWS_AS(ms.peekInt32(), std::runtime_error);

  ms.clear();
  ms.writeInt8(123);
  CHECK(ms.peekInt8() == 123);
  CHECK(ms.peekInt8() == 123);

  ms.clear();
  ms.writeInt8(100);
  CHECK(ms.peekInt8() == 100);
  CHECK(ms.peekInt8() == 100);

  ms.clear();
  ms.writeUInt16(0x1122);
  CHECK(ms.peekInt16() == 0x1122);
  CHECK(ms.peekInt16() == 0x1122);

  ms.clear();
  ms.writeInt16(0x3456);
  CHECK(ms.peekInt16() == 0x3456);
  CHECK(ms.peekInt16() == 0x3456);

  ms.clear();
  ms.writeUInt32(0xABCD1234);
  CHECK(ms.peekInt32() == 0xABCD1234);
  CHECK(ms.peekInt32() == 0xABCD1234);

  ms.clear();
  ms.writeInt32(0x12345678);
  CHECK(ms.peekInt32() == 0x12345678);
  CHECK(ms.peekInt32() == 0x12345678);

  ms.clear();
  ms.writeDouble(5.0);
  CHECK(ms.peekDouble() == 5.0);
  CHECK(ms.peekDouble() == 5.0);

  ms.clear();
  ms.writeString("LOL-42");
  CHECK(ms.peekString() == "LOL-42");
  CHECK(ms.peekString() == "LOL-42");
}

TEST_CASE("MemoryStream: read", "[MemoryStream]")
{
  MemoryStream ms;
  ms.writeUInt32(0xAABBCCDD);
  ms.writeUInt32(0x12345678);
  ms.writeString("LOL");

  CHECK(ms.readUInt32() == 0xAABBCCDD);
  CHECK(ms.readUInt32() == 0x12345678);
  CHECK(ms.readString() == "LOL");
  CHECK(ms.length() == 13);
  CHECK(ms.tellG() == 13);
  CHECK(ms.tellP() == 13);
  CHECK_THROWS_AS(ms.readUInt32(), std::runtime_error);

  ms.clear();
  ms.writeInt16(42);
  CHECK_THROWS_AS(ms.readString(), std::runtime_error);

  ms.writeInt8(123);
  CHECK(ms.readInt8() == 123);
  CHECK_THROWS_AS(ms.readInt8(), std::runtime_error);

  ms.writeUInt8(0xFE);
  CHECK(ms.readUInt8() == 0xFE);
  CHECK_THROWS_AS(ms.readInt8(), std::runtime_error);

  ms.writeInt16(0x1234);
  CHECK(ms.readInt16() == 0x1234);
  CHECK_THROWS_AS(ms.readInt8(), std::runtime_error);

  ms.writeUInt16(0xFEDC);
  CHECK(ms.readUInt16() == 0xFEDC);
  CHECK_THROWS_AS(ms.readInt8(), std::runtime_error);

  ms.writeInt32(0x12345678);
  CHECK(ms.readInt32() == 0x12345678);
  CHECK_THROWS_AS(ms.readInt8(), std::runtime_error);

  ms.writeUInt32(0xFEDCBA98);
  CHECK(ms.readUInt32() == 0xFEDCBA98);
  CHECK_THROWS_AS(ms.readInt8(), std::runtime_error);

  ms.clear();
  ms.writeFloat(42.0);
  CHECK(ms.readFloat() == 42.0);
  CHECK_THROWS_AS(ms.readInt8(), std::runtime_error);

  ms.clear();
  ms.writeDouble(5.0);
  CHECK(ms.readDouble() == 5.0);
  CHECK_THROWS_AS(ms.readInt8(), std::runtime_error);

  ms.clear();
  ms.writeString("LOL-42");
  CHECK(ms.readString() == "LOL-42");
  CHECK_THROWS_AS(ms.readInt8(), std::runtime_error);
}

TEST_CASE("MemoryStream: resize", "[MemoryStream]")
{
  MemoryStream ms;

  CHECK(ms.data() == nullptr);
  CHECK(ms.length() == 0);
  CHECK(ms.tellG() == 0);
  CHECK(ms.tellP() == 0);

  ms.resize(100);
  CHECK(ms.data() != nullptr);
  CHECK(ms.length() == 100);
  CHECK(ms.tellG() == 0);
  CHECK(ms.tellP() == 0);

  ms.seekG(0, Direction::End);
  ms.seekP(-25, Direction::End);
  CHECK(ms.tellG() == 100);
  CHECK(ms.tellP() == 75);

  ms.resize(50);
  CHECK(ms.length() == 50);
  CHECK(ms.tellG() == 50);
  CHECK(ms.tellP() == 50);

  ms.resize(0);
  CHECK(ms.data() == nullptr);
  CHECK(ms.length() == 0);
  CHECK(ms.tellG() == 0);
  CHECK(ms.tellP() == 0);
}

TEST_CASE("MemoryStream: seek", "[MemoryStream]")
{
  MemoryStream ms;
  ms.writeUInt32(42);

  CHECK(ms.availableForRead() == 4);
  CHECK(ms.availableForWrite() == 256 - 4);
  ms.seekG(0, Direction::End);
  ms.seekP(0, Direction::Begin);
  CHECK(ms.availableForRead() == 0);
  CHECK(ms.availableForWrite() == 256);

  CHECK(ms.seekG(-1, Direction::Begin) == 4);
  CHECK(ms.seekG(11, Direction::End) == 0);
  CHECK(ms.seekG(-3, Direction::Current) == 15);
  CHECK(ms.seekG(1, Direction::Current) == 12);
  CHECK(ms.tellG() == 13);

  CHECK(ms.seekP(-1, Direction::Begin) == 0);
  CHECK(ms.seekP(0, Direction::End) == 0);
  CHECK(ms.seekP(-3, Direction::Current) == 15);
  CHECK(ms.seekP(1, Direction::Current) == 12);
  CHECK(ms.tellP() == 13);

  ms.seekG(1000, Direction::End);
  CHECK(ms.length() == 1015);
  CHECK(ms.tellG() == 1015);
  CHECK(ms.tellP() == 13);
}

TEST_CASE("MemoryStream: write", "[MemoryStream]")
{
  MemoryStream ms;

  ms.writeUInt8(42);
  CHECK(ms.tellP() == 1);
  CHECK(ms.length() == 1);

  ms.writeUInt16(42);
  CHECK(ms.tellP() == 3);
  CHECK(ms.length() == 3);

  ms.seekP(1, Direction::Begin);
  ms.writeUInt32(42);
  CHECK(ms.tellP() == 5);
  CHECK(ms.length() == 5);

  ms.seekP(0, Direction::Begin);
  ms.writeUInt32(2012);
  CHECK(ms.tellP() == 4);
  CHECK(ms.length() == 5);

  char buff[15];
  ms.write(buff, 15);
  CHECK(ms.tellP() == 19);
  CHECK(ms.length() == 19);

  ms.writeString("LOL2012");
  CHECK(ms.tellP() == 28);
  CHECK(ms.length() == 28);

  MemoryStream ms2;
  ms2.writeUInt32(0x11223344);
  ms2.write(ms.data(), 8);
  CHECK(ms2.tellP() == 12);
  CHECK(ms2.length() == 12);

  CHECK(ms2.readUInt32() == 0x11223344);
  CHECK(ms2.readUInt32() == 2012);
  CHECK(ms2.tellG() == 8);
}

TEST_CASE("MemoryStream: fill", "[MemoryStream]")
{
  const int n = 10000;
  MemoryStream ms;
  ms.reserve(n * sizeof(int));

  for (int i=0; i<n; ++i) {
    ms.writeInt32(i);
  }
  CHECK(ms.length() == n * sizeof(int));

  bool f = true;
  for (int i=0; i<n; ++i) {
    if (ms.readInt32() != i) {
      f = false;
      break;
    }
  }
  CHECK(f);
}