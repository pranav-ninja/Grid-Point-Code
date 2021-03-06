/*
 * Copyright 2017 Pranavkumar Patel
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "GPC.h"

namespace GridPointCode {

	const string GPC::CHARACTERS = "0123456789CDFGHJKLMNPRTVWXY";	//base27

	GPC::Coordinates::Coordinates(double latitude, double longitude) : Latitude(latitude), Longitude(longitude) {

	}

	GPC::CoordinateSeven::CoordinateSeven(vector<int> &latitudeSeven, vector<int> &longitudeSeven) {
		LatitudeSeven = latitudeSeven;
		LongitudeSeven = longitudeSeven;
	}

	GPC::CoordinateSignWhole::CoordinateSignWhole(int latitudeSign, int latitudeWhole, int longitudeSign, int longitudeWhole) {
		LatitudeSign = latitudeSign;
		LatitudeWhole = latitudeWhole;
		LongitudeSign = longitudeSign;
		LongitudeWhole = longitudeWhole;
	}
	
	/*  PART 1 : ENCODE */
	
	//Get a Grid Point Code
	string GPC::GetGridPointCode(double latitude, double longitude) {
		return GetGridPointCode(latitude, longitude, true);
	}

	string GPC::GetGridPointCode(double latitude, double longitude, bool formatted) {
		double Latitude = latitude; //Latitude
		double Longitude = longitude; //Longitude

		/*  Validating Latitude and Longitude values */
		if (Latitude <= -90 || Latitude >= 90)
		{
			throw invalid_argument("Latitude value must be between -90 to 90.");
		}
		if (Longitude <= -180 || Longitude >= 180)
		{
			throw invalid_argument("Longitude value must be between -180 to 180.");
		}

		/*  Getting a Point Number  */
		unsigned long long Point = GetPointNumber(Latitude, Longitude);

		/*  Encode Point    */
		string GridPointCode = EncodePoint(Point + ELEVEN);

		/*  Format GridPointCode   */
		if (formatted)
		{
			GridPointCode = FormatGPC(GridPointCode);
		}

		return GridPointCode;
	}

	//Get Point from Coordinates
	unsigned long long GPC::GetPointNumber(double latitude, double longitude) {
		vector<int> LatitudeSeven = GetCoordinateSeven(latitude);
		vector<int> LongitudeSeven = GetCoordinateSeven(longitude);

		//Whole-Number Part
		CoordinateSignWhole SignWhole = CoordinateSignWhole(LatitudeSeven[0], LatitudeSeven[1], LongitudeSeven[0], LongitudeSeven[1]);
		unsigned long long Point = static_cast<unsigned long long>(pow(10, 10) * GetCombinationNumber(SignWhole));

		//Fractional Part
		int Power = 9;
		for (int index = 2; index <= 6; index++)
		{
			Point = Point + static_cast<unsigned long long>(pow(10, Power--) * LongitudeSeven[index]);
			Point = Point + static_cast<unsigned long long>(pow(10, Power--) * LatitudeSeven[index]);
		}
		return Point;
	}

	//Break down coordinate into seven parts
	vector<int> GPC::GetCoordinateSeven(double coordinate) {
		vector<int> Result(7);
		Result[0] = (coordinate < 0 ? -1 : 1);	//Sign
		Result[1] = static_cast<int>(trunc(abs(coordinate)));	//Whole-Number

		int Fractional = static_cast<int>(trunc(abs(coordinate) * pow(10, 5)));	//Fractional
		for (int x = 1, y = 5; x <= 5; x++, y--) {
			Result[x + 1] = ((Fractional % static_cast<int>(pow(10, y)))
				- (Fractional % static_cast<int>(pow(10, y - 1))))
				/ static_cast<int>(pow(10, y - 1));
		}
		return Result;
	}

	//Get Combination Number of Whole-Numbers
	int GPC::GetCombinationNumber(CoordinateSignWhole signWhole) {
		int AssignedLongitude = (signWhole.LongitudeWhole * 2) + (signWhole.LongitudeSign == -1 ? 1 : 0);
		int AssignedLatitude = (signWhole.LatitudeWhole * 2) + (signWhole.LatitudeSign == -1 ? 1 : 0);

		int MaxSum = 538;
		int Sum = AssignedLongitude + AssignedLatitude;
		int Combination = 0;

		if (Sum <= 179) {
			for (int xSum = (Sum - 1); xSum >= 0; xSum--) {
				Combination += xSum + 1;
			}
			Combination += AssignedLongitude + 1;
		}
		else if (Sum <= 359) {
			Combination += 16290;
			Combination += (Sum - 180) * 180;
			Combination += 180 - AssignedLatitude;
		}
		else if (Sum <= 538) {
			Combination += 48690;
			for (int xSum = (Sum - 1); xSum >= 360; xSum--) {
				Combination += MaxSum - xSum + 1;
			}
			Combination += 180 - AssignedLatitude;
		}
		return Combination;
	}

	//Encode Point to GPC
	string GPC::EncodePoint(unsigned long long point) {
		unsigned long long Point = point;
		string Result = "";
		unsigned long long Base = static_cast<unsigned long long>(CHARACTERS.length());

		if (Point == 0) {
			Result += CHARACTERS[0];
		}
		else {
			while (Point > 0) {
				Result = CHARACTERS.substr(Point % Base, 1) + Result;
				Point /= Base;
			}
		}
		return Result;
	}

	//Format GPC
	string GPC::FormatGPC(const string &gridPointCode) {
		return "#" + gridPointCode.substr(0, 4) + "-" + gridPointCode.substr(4, 4) + "-" + gridPointCode.substr(8, 3);
	}

	/*  PART 2 : DECODE */

	//Get Coordinates from GPC
	GPC::Coordinates GPC::GetCoordinates(string gridPointCode) {
		/*  Unformatting and Validating GPC  */
		string GridPointCode = UnformatNValidateGPC(gridPointCode);

		/*  Getting a Point Number  */
		unsigned long long Point = DecodeToPoint(GridPointCode) - ELEVEN;

		/* Getting Coordinates from Point  */
		return GetCoordinates(Point);
	}
	
	//Remove format and validate GPC
	string GPC::UnformatNValidateGPC(string gridPointCode) {
		transform(gridPointCode.begin(), gridPointCode.end(), gridPointCode.begin(), ::toupper);

		string Grid = "";
		for (char character : gridPointCode) {
			if (CHARACTERS.find(character) != string::npos) {
				Grid = Grid + character;
			}
		}

		if (Grid.length() != 11) {
			throw invalid_argument("Length of GPC must be 11.");
		}
		return Grid;
	}

	//Decode string to Point
	unsigned long long GPC::DecodeToPoint(const string &gridPointCode) {
		unsigned long long Result = 0;
		unsigned long long Base = static_cast<unsigned long long>(CHARACTERS.length());

		for (string::size_type i = 0; i < gridPointCode.length(); i++) {
			Result *= Base;
			char character = gridPointCode[i];
			Result += static_cast<int>(CHARACTERS.find(character));
		}
		return Result;
	}

	//Get a Coordinates from Point
	GPC::Coordinates GPC::GetCoordinates(unsigned long long point) {
		int Combination = static_cast<int>(trunc((point / pow(10, 10))));
		unsigned long long Fractional = static_cast<unsigned long long>(point - (Combination * pow(10, 10)));
		return GetCoordinates(GetCoordinateSeven(Combination, Fractional));
	}

	//Combine Seven Parts to Coordinate
	GPC::Coordinates GPC::GetCoordinates(CoordinateSeven CSeven) {
		int Power = 0;
		int TempLatitude = 0;
		int TempLongitude = 0;
		for (int x = 6; x >= 1; x--) {
			TempLatitude += static_cast<int>(CSeven.LatitudeSeven[x] * pow(10, Power));
			TempLongitude += static_cast<int>(CSeven.LongitudeSeven[x] * pow(10, Power++));
		}

		double Latitude = (TempLatitude / pow(10, 5)) * CSeven.LatitudeSeven[0];
		double Longitude = (TempLongitude / pow(10, 5)) * CSeven.LongitudeSeven[0];
		return Coordinates(Latitude, Longitude);
	}

	//Get Seven Parts of Coordinate
	GPC::CoordinateSeven GPC::GetCoordinateSeven(int combination, unsigned long long fractional) {
		vector<int> LongitudeSeven(7), LatitudeSeven(7);

		CoordinateSignWhole SignWhole = GetWholesFromCombination(combination);
		LongitudeSeven[0] = SignWhole.LongitudeSign;
		LongitudeSeven[1] = SignWhole.LongitudeWhole;
		LatitudeSeven[0] = SignWhole.LatitudeSign;
		LatitudeSeven[1] = SignWhole.LatitudeWhole;

		int Power = 9;
		for (int x = 2; x <= 6; x++) {
			LongitudeSeven[x] = static_cast<int>((static_cast<unsigned long long>(trunc(fractional / pow(10, Power--)))) % 10);
			LatitudeSeven[x] = static_cast<int>((static_cast<unsigned long long>(trunc(fractional / pow(10, Power--)))) % 10);
		}
		return CoordinateSeven(LatitudeSeven, LongitudeSeven);
	}

	//Get Whole-Numbers from Combination number
	GPC::CoordinateSignWhole GPC::GetWholesFromCombination(int combination)
	{
		int MaxSum = 538;

		int XSum = 0;
		int XCombination = 0;

		int Sum = 0;
		int MaxCombination = 0;
		int AssignedLongitude = 0;
		int AssignedLatitude = 0;

		if (combination <= 16290) {
			for (XSum = 0; MaxCombination < combination; XSum++) {
				MaxCombination += XSum + 1;
			}
			Sum = XSum - 1;
			XCombination = MaxCombination - (Sum + 1);

			AssignedLongitude = combination - XCombination - 1;
			AssignedLatitude = Sum - AssignedLongitude;
		}
		else if (combination <= 48690) {
			XCombination = 16290;
			XSum = 179;

			bool IsLast = (combination - XCombination) % 180 == 0;
			int Pre = ((combination - XCombination) / 180) - (IsLast ? 1 : 0);

			XSum += Pre;
			XCombination += Pre * 180;

			Sum = XSum + 1;
			AssignedLatitude = 180 - (combination - XCombination);
			AssignedLongitude = Sum - AssignedLatitude;
		}
		else if (combination <= 64800) {
			XCombination = 48690;
			XSum = 359;
			MaxCombination += XCombination;
			for (XSum = 360; MaxCombination < combination; XSum++) {
				MaxCombination += (MaxSum - XSum + 1);
			}
			Sum = XSum - 1;
			XCombination = MaxCombination - (MaxSum - Sum + 1);

			AssignedLatitude = 180 - (combination - XCombination);
			AssignedLongitude = Sum - AssignedLatitude;
		}

		CoordinateSignWhole Result = CoordinateSignWhole();
		Result.LongitudeSign = (AssignedLongitude % 2 != 0 ? -1 : 1);
		Result.LongitudeWhole = (Result.LongitudeSign == -1 ? (--AssignedLongitude / 2) : (AssignedLongitude / 2));
		Result.LatitudeSign = (AssignedLatitude % 2 != 0 ? -1 : 1);
		Result.LatitudeWhole = (Result.LatitudeSign == -1 ? (--AssignedLatitude / 2) : (AssignedLatitude / 2));
		return Result;
	}
}