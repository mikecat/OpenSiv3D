﻿//-----------------------------------------------
//
//	This file is part of the Siv3D Engine.
//
//	Copyright (C) 2008-2016 Ryo Suzuki
//	Copyright (C) 2016 OpenSiv3D Project
//
//	Licensed under the MIT License.
//
//-----------------------------------------------

# include <Siv3D/Logger.hpp>
# include "../Siv3DEngine.hpp"
# include "ILogger.hpp"

namespace s3d
{
	namespace detail
	{
		void Log_impl::operator()(const String& text) const
		{
			Siv3DEngine::GetLogger()->write(LogDescription::App, text);
		}
	}
}