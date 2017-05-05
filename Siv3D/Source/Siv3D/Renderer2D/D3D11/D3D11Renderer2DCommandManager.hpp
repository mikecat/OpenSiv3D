﻿//-----------------------------------------------
//
//	This file is part of the Siv3D Engine.
//
//	Copyright (c) 2008-2017 Ryo Suzuki
//	Copyright (c) 2016-2017 OpenSiv3D Project
//
//	Licensed under the MIT License.
//
//-----------------------------------------------

# pragma once
# include <Siv3D/Platform.hpp>
# if defined(SIV3D_TARGET_WINDOWS)

# define  NOMINMAX
# define  STRICT
# define  WIN32_LEAN_AND_MEAN
# define  _WIN32_WINNT _WIN32_WINNT_WIN7
# define  NTDDI_VERSION NTDDI_WIN7
# include <Windows.h>
# include <d3d11.h>
# include <Siv3D/Fwd.hpp>
# include <Siv3D/Array.hpp>
# include <Siv3D/Byte.hpp>
# include <Siv3D/BlendState.hpp>
# include <Siv3D/Rectangle.hpp>
# include <Siv3D/Texture.hpp>
# include <Siv3D/RenderTexture.hpp>
# include <Siv3D/HashMap.hpp>
# include "../../Siv3DEngine.hpp"
# include "../../Graphics/IGraphics.hpp"

namespace s3d
{
	enum class D3D11Render2DInstruction : uint16
	{
		Nop,

		Draw,

		NextBatch,

		BlendState,

		Viewport,

		RenderTarget,
	};

	struct D3D11Render2DCommandHeader
	{
		D3D11Render2DInstruction instruction;

		uint16 commandSize;
	};

	template <D3D11Render2DInstruction instruction>
	struct D3D11Render2DCommand;

	template <>
	struct D3D11Render2DCommand<D3D11Render2DInstruction::Draw>
	{
		D3D11Render2DCommandHeader header =
		{
			D3D11Render2DInstruction::Draw,

			sizeof(D3D11Render2DCommand<D3D11Render2DInstruction::Draw>)
		};

		uint32 indexSize;
	};

	template <>
	struct D3D11Render2DCommand<D3D11Render2DInstruction::NextBatch>
	{
		D3D11Render2DCommandHeader header =
		{
			D3D11Render2DInstruction::NextBatch,

			sizeof(D3D11Render2DCommand<D3D11Render2DInstruction::NextBatch>)
		};
	};

	template <>
	struct D3D11Render2DCommand<D3D11Render2DInstruction::BlendState>
	{
		D3D11Render2DCommandHeader header =
		{
			D3D11Render2DInstruction::BlendState,

			sizeof(D3D11Render2DCommand<D3D11Render2DInstruction::BlendState>)
		};

		BlendState blendState;
	};

	template <>
	struct D3D11Render2DCommand<D3D11Render2DInstruction::Viewport>
	{
		D3D11Render2DCommandHeader header =
		{
			D3D11Render2DInstruction::Viewport,

			sizeof(D3D11Render2DCommand<D3D11Render2DInstruction::Viewport>)
		};

		Optional<Rect> viewport;
	};

	template <>
	struct D3D11Render2DCommand<D3D11Render2DInstruction::RenderTarget>
	{
		D3D11Render2DCommandHeader header =
		{
			D3D11Render2DInstruction::RenderTarget,

			sizeof(D3D11Render2DCommand<D3D11Render2DInstruction::RenderTarget>)
		};

		Texture::IDType textureID;
	};

	class D3D11Render2DCommandManager
	{
	private:

		Array<Byte> m_commands;

		size_t m_commandCount = 0;

		Byte* m_lastCommandPointer = nullptr;

		D3D11Render2DInstruction m_lastCommand = D3D11Render2DInstruction::Nop;

		BlendState m_currentBlendState = BlendState::Default;

		Optional<Rect> m_currentViewport;

		HashMap<Texture::IDType, Texture> m_reservedTextures;

		RenderTexture m_currentRenderTarget;

		template <class Command>
		void writeCommand(const Command& command)
		{
			static_assert(sizeof(Command) % 4 == 0);
			
			const Byte* dataBegin = static_cast<const Byte*>(static_cast<const void*>(&command));

			const Byte* dataEnd = dataBegin + sizeof(Command);

			m_commands.insert(m_commands.end(), dataBegin, dataEnd);

			++m_commandCount;

			m_lastCommandPointer = m_commands.data() + (m_commands.size() - sizeof(Command));

			m_lastCommand = command.header.instruction;
		}

		template <D3D11Render2DInstruction instruction>
		D3D11Render2DCommand<instruction>& getLastCommand()
		{
			return *static_cast<D3D11Render2DCommand<instruction>*>(static_cast<void*>(m_lastCommandPointer));
		}

	public:

		size_t getCount() const
		{
			return m_commandCount;
		}

		const Byte* getCommandBuffer() const
		{
			return m_commands.data();
		}

		void reset()
		{
			m_commands.clear();

			m_reservedTextures.clear();

			m_commandCount = 0;

			pushNextBatch();

			{
				D3D11Render2DCommand<D3D11Render2DInstruction::BlendState> command;
				command.blendState = m_currentBlendState;
				writeCommand(command);
			}

			{
				const RenderTexture& backBuffer2D = Siv3DEngine::GetGraphics()->getBackBuffer2D();
				m_currentRenderTarget = backBuffer2D;
				m_reservedTextures.emplace(backBuffer2D.id(), backBuffer2D);

				D3D11Render2DCommand<D3D11Render2DInstruction::RenderTarget> command;
				command.textureID = backBuffer2D.id();
				writeCommand(command);
			}

			{
				D3D11Render2DCommand<D3D11Render2DInstruction::Viewport> command;
				command.viewport = m_currentViewport;
				writeCommand(command);
			}
		}

		void pushDraw(const uint32 indexSize)
		{
			if (m_lastCommand == D3D11Render2DInstruction::Draw)
			{
				getLastCommand<D3D11Render2DInstruction::Draw>().indexSize += indexSize;
				
				return;
			}

			D3D11Render2DCommand<D3D11Render2DInstruction::Draw> command;
			command.indexSize = indexSize;
			writeCommand(command);
		}

		void pushNextBatch()
		{
			writeCommand(D3D11Render2DCommand<D3D11Render2DInstruction::NextBatch>());
		}

		void pushBlendState(const BlendState& state)
		{
			if (state == m_currentBlendState)
			{
				return;
			}

			D3D11Render2DCommand<D3D11Render2DInstruction::BlendState> command;
			command.blendState = state;
			writeCommand(command);

			m_currentBlendState = state;
		}

		const BlendState& getCurrentBlendState() const
		{
			return m_currentBlendState;
		}

		void pushViewport(const Optional<Rect>& viewport)
		{
			if (viewport == m_currentViewport)
			{
				return;
			}

			D3D11Render2DCommand<D3D11Render2DInstruction::Viewport> command;
			command.viewport = viewport;
			writeCommand(command);

			m_currentViewport = viewport;
		}

		Optional<Rect> getCurrentViewport() const
		{
			return m_currentViewport;
		}

		void pushRenderTarget(const RenderTexture& texture)
		{
			const auto id = texture.id();

			if (id == m_currentRenderTarget.id())
			{
				return;
			}

			if (m_reservedTextures.find(id) == m_reservedTextures.end())
			{
				m_reservedTextures.emplace(id, texture);
			}

			D3D11Render2DCommand<D3D11Render2DInstruction::RenderTarget> command;
			command.textureID = id;
			writeCommand(command);

			m_currentRenderTarget = texture;
		}
	};
}

# endif