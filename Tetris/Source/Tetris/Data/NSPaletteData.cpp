#include "Data/NSPaletteData.h"

namespace NSPaletteDefaults
{
	static FLinearColor Hex(const TCHAR* H) { return FLinearColor::FromSRGBColor(FColor::FromHex(H)); }

	static FNSPalette Make(const TCHAR* Name, const TCHAR* SkyTop, const TCHAR* SkyHorizon,
		const TCHAR* SunTop, const TCHAR* SunBottom, const TCHAR* Grid, const TCHAR* Accent)
	{
		FNSPalette P;
		P.Name = FName(Name);
		P.SkyTop = Hex(SkyTop);
		P.SkyHorizon = Hex(SkyHorizon);
		P.SunTop = Hex(SunTop);
		P.SunBottom = Hex(SunBottom);
		P.Grid = Hex(Grid);
		P.Accent = Hex(Accent);
		return P;
	}
}

UNSPaletteData::UNSPaletteData()
{
	using namespace NSPaletteDefaults;
	// art-bible §4.3: céu (topo -> horizonte), sol (topo -> base), grade, acento de UI
	Palettes.Add(Make(TEXT("Sunset"),   TEXT("12002B"), TEXT("5A0F5E"), TEXT("FF7A3D"), TEXT("FF2E88"), TEXT("FF2E88"), TEXT("FF3DCB")));
	Palettes.Add(Make(TEXT("Midnight"), TEXT("000814"), TEXT("0A2A5E"), TEXT("2DE2FF"), TEXT("3D7BFF"), TEXT("2DE2FF"), TEXT("2DE2FF")));
	Palettes.Add(Make(TEXT("Toxic"),    TEXT("00110A"), TEXT("0B3D2E"), TEXT("9CFF4A"), TEXT("1FD1A5"), TEXT("1FD1A5"), TEXT("9CFF4A")));
	Palettes.Add(Make(TEXT("Inferno"),  TEXT("140000"), TEXT("4A0A00"), TEXT("FFC23D"), TEXT("FF3B1F"), TEXT("FF5E5B"), TEXT("FFC23D")));
}

const FNSPalette& UNSPaletteData::GetPalette(int32 Index) const
{
	static const FNSPalette Fallback;
	if (Palettes.Num() == 0)
	{
		return Fallback;
	}
	return Palettes[((Index % Palettes.Num()) + Palettes.Num()) % Palettes.Num()];
}
