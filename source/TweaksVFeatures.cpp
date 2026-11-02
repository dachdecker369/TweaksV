// SPDX-License-Identifier: GPL-3.0-only
// TweaksV - its features (see TweaksVFeatures.h).

#include "TweaksVFeatures.h"

#include <windows.h>

#include <initializer_list>
#include <iterator>

#include "TweaksVBlips.h"
#include "TweaksVBoxStreamer.h"
#include "TweaksVDecals.h"
#include "TweaksVDirectStorage.h"
#include "TweaksVHeapAdjuster.h"
#include "TweaksVLightCapacity.h"
#include "TweaksVLog.h"
#include "TweaksVModkitLimit.h"
#include "TweaksVPackfileLimit.h"
#include "TweaksVPedPool.h"
#include "TweaksVPedProps.h"
#include "TweaksVPopulation.h"
#include "TweaksVRageOpenV.h"
#include "TweaksVRayTracing.h"
#include "TweaksVRenderQueues.h"
#include "TweaksVSirenCount.h"
#include "TweaksVSirenIds.h"
#include "TweaksVStaticBounds.h"
#include "TweaksVStraightToStoryMode.h"
#include "TweaksVSummary.h"
#include "TweaksVTextureBudget.h"
#include "TweaksVWeaponComponents.h"
#include "TweaksVWeaponInfos.h"

namespace TweaksV
{
	namespace
	{
		// NaturalVision Enhanced's mods loader does RageOpenV's job and skips the splash and legal screens
		// by its own INI, as StraightToStoryMode does. It loads before TweaksV but patches those sites after
		// it, at the game's first request for the time, uses what it finds there unchecked and faults when
		// TweaksV has changed them. NVE needs it, so it stays, and those two parts are off while it is there.
		constexpr PeerPlugin kNveLoader{ L"onigiri.asi", "NaturalVision Enhanced's loader" };

		// Resource Extensions [Enhanced] by Koukol, closed source, raises many of the game's capacities - its page
		// names the archive count, the story blips' threshold and extra capacity for the ray tracing of moving geometry
		// among them and asks to remove the heap and packfile adjusters. It stays for the jobs TweaksV does not do, so
		// the four parts whose jobs it does too are off while it is there, and nothing is patched twice.
		constexpr PeerPlugin kResourceExtensions{ L"ResourceExtensions.asi", "Resource Extensions [Enhanced]" };

		// The five original plugins in the order an ASI loader loaded their .asi files (alphabetical),
		// TweaksV's own box streamer and decal definitions parts among the limit adjusters, then its
		// DirectStorage part, its Blips part, the four parts of the parts' coding round and the two of phase C. ZHeapAdjuster.asi
		// is the rename the HeapAdjuster author recommends; OpenRPF.asi and OpenIV.asi do RageOpenV's job,
		// alloc8or's DirectStorageFix.asi that of the DirectStorage part, rootBrz's BlipLimitFix.asi that of
		// the Blips part, Nochala's Static Bounds Limit Adjuster - StaticBoundLimitAdjuster.asi on its page,
		// StaticBoundsLimitAdjuster.asi in its repository - that of the StaticBounds part,
		// WeaponLimitsAdjusterEnhanced.asi that of the WeaponLimits part, GTAV.ResourceAdjusterEnhanced.asi that of
		// the Resources part, and the
		// closed-source DecalPatch.asi that of both the box streamer and the decal definitions parts, so it
		// switches both off. WeaponLimits is two modules, each started in turn.
		constexpr Feature kFeatures[] = {
			{ &kHeapSection, { L"HeapAdjuster.asi", L"ZHeapAdjuster.asi" }, { &kResourceExtensions },
				[] { InitHeapAdjuster(); return false; } },
			{ &kModkitSection, { L"ModkitLimitAdjusterEnhanced.asi" }, {},
				[] { InitModkitLimit(); return false; } },
			{ &kPackfileSection, { L"PackfileLimitAdjusterEnhanced.asi" }, { &kResourceExtensions },
				[] { InitPackfileLimit(); return false; } },
			{ &kBoxStreamerSection, { L"DecalPatch.asi" }, {},
				[] { InitBoxStreamer(); return false; } },
			{ &kDecalsSection, { L"DecalPatch.asi" }, {},
				[] { InitDecals(); return false; } },
			// RageOpenV's start-up runs later, in its GetSystemTimeAsFileTime hook, which then shows the summary
			{ &kRageOpenVSection, { L"RageOpenV.asi", L"OpenRPF.asi", L"OpenIV.asi" }, { &kNveLoader },
				[] { return InitRageOpenV(); } },
			{ &kStoryModeSection, { L"StraightToStoryMode.asi" }, { &kNveLoader },
				[] { InitStraightToStoryMode(); return false; } },
			{ &kDirectStorageSection, { L"DirectStorageFix.asi" }, {},
				[] { InitDirectStorage(); return false; } },
			{ &kBlipsSection, { L"BlipLimitFix.asi" }, { &kResourceExtensions },
				[] { InitBlips(); return false; } },
			// WeaponLimits is two modules, each done on its own (SPECIFICATIONS 11.10): each start has its own try, so that an
			// exception ends only that half, which the problem names, and the other still starts.
			{ &kWeaponLimitsSection, { L"WeaponLimitsAdjusterEnhanced.asi" }, {},
				[] {
#ifdef TWEAKSV_TEST_BUILD
					// test builds only: TWEAKSV_TEST_WEAPON_HALF_THROWS=1 or 2 in the environment makes that half throw in place of
					// starting, so that a suite sees its problem and the other half start
					wchar_t throwing[4]{};
					if (GetEnvironmentVariableW(L"TWEAKSV_TEST_WEAPON_HALF_THROWS", throwing, 4) != 1)
						throwing[0] = L'\0';
#endif
					try
					{
#ifdef TWEAKSV_TEST_BUILD
						if (throwing[0] == L'1')
						{
							WriteLog(kWeaponLimitsSection.feature, "TEST BUILD: the weapon infos' half throws in place of starting (TWEAKSV_TEST_WEAPON_HALF_THROWS=1).");
							throw 1;
						}
#endif
						InitWeaponInfos();
					}
					catch (...)
					{
						ReportFailure(kWeaponLimitsSection.feature, "The weapon infos' half is not done this session: an unexpected exception ended it.");
					}
					try
					{
#ifdef TWEAKSV_TEST_BUILD
						if (throwing[0] == L'2')
						{
							WriteLog(kWeaponLimitsSection.feature, "TEST BUILD: the weapon components' half throws in place of starting (TWEAKSV_TEST_WEAPON_HALF_THROWS=2).");
							throw 2;
						}
#endif
						InitWeaponComponents();
					}
					catch (...)
					{
						ReportFailure(kWeaponLimitsSection.feature, "The weapon components' half is not done this session: an unexpected exception ended it.");
					}
					return false;
				} },
			{ &kPedPropsSection, {}, {},
				[] { InitPedProps(); return false; } },
			{ &kResourcesSection, { L"GTAV.ResourceAdjusterEnhanced.asi" }, {},
				[] { InitTextureBudget(); return false; } },
			{ &kRayTracingSection, {}, { &kResourceExtensions },
				[] { InitRayTracing(); return false; } },
			{ &kSirenSettingsSection, {}, {},
				[] { InitSirenIds(); return false; } },
			{ &kStaticBoundsSection, { L"StaticBoundLimitAdjuster.asi", L"StaticBoundsLimitAdjuster.asi" }, {},
				[] { InitStaticBounds(); return false; } },
		};

		// Every section of TweaksV.ini but [TWEAKSV] belongs to a feature, in the same order.
		static_assert(std::size(kFeatures) + 1 == std::size(kSections), "TweaksV: kFeatures needs one entry per section but [TWEAKSV]");
		consteval bool InSectionOrder()
		{
			for (size_t i = 0; i < std::size(kFeatures); i++)
				if (kFeatures[i].section != kSections[i + 1])
					return false;
			return true;
		}
		static_assert(InSectionOrder(), "TweaksV: kFeatures must be in the order of kSections");

		// A limit of a part to come, measured before its part writes anything (SPECIFICATIONS 11.16): with
		// DIAGNOSTIC_MODE=1 its module finds and checks the sites of the limit and measures it.
		struct MeasuredFirst
		{
			const char* feature;	// the part's log tag: "WeaponLimits"
			void (*start)();		// the module's Measure<Limit>()
		};

		// The parts measured first, one entry per limit in the order of SPECIFICATIONS 11.16, and last the ped props'
		// measurement, which goes on while that part, which ships off, is off (11.11). A limit leaves the table when
		// its part is built, or when it is not, so the table may be empty.
		constexpr std::initializer_list<MeasuredFirst> kMeasuredFirst = {
			// the siren IDs' measurement goes on while their job, LARGE_SIREN_IDS, is off, for a start with it and one without (11.14)
			{ "SirenSettings", MeasureSirenIdsWhileOff },
			// [siren count]
			{ "SirenSettings", MeasureSirenCount },
			{ "LightCapacity", MeasureLightCapacity },
			{ "Population", MeasurePopulation },
			{ "PedPool", MeasurePedPool },
			{ "RenderQueues", MeasureRenderQueues },
			{ "PedProps", MeasurePedPropsWhileOff },
		};

#ifdef TWEAKSV_TEST_BUILD
		// test builds only: TWEAKSV_TEST_THROWS=<what> in the environment makes what it names throw in place of starting, so that a
		// suite reaches the catch-all around it: a part by its tag (RageOpenV), a measurement by its part's tag after "measurement "
		// (measurement LightCapacity; SirenSettings names its two), and the start of them all by "start-up", which only DllMain's catches.
		void ThrowIfNamed(const std::string& what, const char* tag)
		{
			wchar_t named[64]{};
			const DWORD length = GetEnvironmentVariableW(L"TWEAKSV_TEST_THROWS", named, 64);
			if (length == 0 || length >= 64 || ToUtf8(named) != what)
				return;
			WriteLog(tag, "TEST BUILD: " + what + " throws in place of starting (TWEAKSV_TEST_THROWS=" + what + ").");
			throw 1;
		}
#endif
	}

	std::span<const Feature> Features()
	{
		return kFeatures;
	}

	bool StartFeatures()
	{
#ifdef TWEAKSV_TEST_BUILD
		ThrowIfNamed("start-up", "TweaksV");
#endif
		bool summaryLater = false;
		for (const Feature& feature : kFeatures)
		{
			if (!IsFeatureOn(*feature.section))
				continue;
			try
			{
#ifdef TWEAKSV_TEST_BUILD
				ThrowIfNamed(feature.section->feature, feature.section->feature);
#endif
				if (feature.start())
					summaryLater = true;
			}
			catch (...)
			{
				ReportFailure(feature.section->feature, "unexpected exception - this part is skipped for this session");
			}
		}
		// The parts measured first, after the parts and with DIAGNOSTIC_MODE=1 only. A measurement is an aid
		// (SPECIFICATIONS 12): an exception ends only that one and is logged, not reported.
		if (IsDiagnosticMode())
			for (const MeasuredFirst& measured : kMeasuredFirst)
			{
				try
				{
#ifdef TWEAKSV_TEST_BUILD
					ThrowIfNamed(std::string("measurement ") + measured.feature, measured.feature);
#endif
					measured.start();
				}
				catch (...)
				{
					WriteLog(measured.feature, "DIAGNOSTIC: unexpected exception - a measurement of this part is skipped for this session.");
				}
			}
		return summaryLater;
	}
}
