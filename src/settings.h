#pragma once

namespace Settings {
	class Holder : public ISingleton<Holder> {
	public:
		bool ReadSettings();
		bool ShouldWarn();

	private:
		bool warn;
	};
}