#pragma once

namespace Settings {
	class Holder : public ISingleton<Holder> {
	public:
		bool ReadSettings();
		bool ShouldWarn();
		bool ShouldSanitize();

	private:
		bool warn;
		bool sanitize;
	};
}