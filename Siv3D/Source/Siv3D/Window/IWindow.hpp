﻿//-----------------------------------------------
//
//	This file is part of the Siv3D Engine.
//
//	Copyright (C) 2008-2017 Ryo Suzuki
//	Copyright (C) 2016-2017 OpenSiv3D Project
//
//	Licensed under the MIT License.
//
//-----------------------------------------------

# pragma once
# include <Siv3D/Fwd.hpp>

# if defined(SIV3D_TARGET_WINDOWS)

	# define  NOMINMAX
	# define  STRICT
	# define  WIN32_LEAN_AND_MEAN
	# define  _WIN32_WINNT _WIN32_WINNT_WIN7
	# define  NTDDI_VERSION NTDDI_WIN7
	# include <Windows.h>

# elif defined(SIV3D_TARGET_MACOS)

	# include "../../ThirdParty/GLFW/include/GLFW/glfw3.h"

# endif

namespace s3d
{
	# if defined(SIV3D_TARGET_WINDOWS)

		using WindowHandle = HWND;

	# else

		using WindowHandle = GLFWwindow*;

	# endif

	class ISiv3DWindow
	{
	public:

		static ISiv3DWindow* Create();

		virtual ~ISiv3DWindow() = default;

		virtual bool init() = 0;
		
		virtual bool update() = 0;

		virtual WindowHandle getHandle() const = 0;

		virtual void setTitle(const String& title, bool forceUpdate = false) = 0;
		
		virtual const WindowState& getState() const = 0;
	};
}