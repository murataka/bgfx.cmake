//
// Created by Murat Aka on 8.06.2022.
//

#include "toc.hpp"


/*
 * Copyright 2011-2022 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bgfx/blob/master/LICENSE
 */

#include <bx/uint32_t.h>

//#include "bgfx_utils.h"
#include "logo.h"
#include "../bgfx/examples/common/entry/entry.h"
#include "../bgfx/examples/common/bgfx_utils.h"
//#include "../bimg/3rdparty/astc/vectypes.h"
#include "bx/timer.h"
//#include "../bgfx/examples/common/imgui/imgui.h"
//#include "imgui/imgui.h"

namespace
{


    struct PosColorVertex
    {
        float m_x;
        float m_y;
        float m_z;
        uint32_t m_abgr;

        static void init()
        {
            ms_layout
                    .begin()
                    .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
                    .add(bgfx::Attrib::Color0,   4, bgfx::AttribType::Uint8, true)
                    .end();
        };

        static bgfx::VertexLayout ms_layout;
    };

    bgfx::VertexLayout PosColorVertex::ms_layout;










    static const uint64_t s_ptState[]
            {
                    UINT64_C(0),
                    BGFX_STATE_PT_TRISTRIP,
                    BGFX_STATE_PT_LINES,
                    BGFX_STATE_PT_LINESTRIP,
                    BGFX_STATE_PT_POINTS,
            };


    static PosColorVertex s_cubeVertices[] =
            {
                    {-1.0f,  1.0f,  1.0f, 0xff000000 },
                    { 1.0f,  1.0f,  1.0f, 0xff0000ff },
                    {-1.0f, -1.0f,  1.0f, 0xff00ff00 },
                    { 1.0f, -1.0f,  1.0f, 0xff00ffff },
                    {-1.0f,  1.0f, -1.0f, 0xffff0000 },
                    { 1.0f,  1.0f, -1.0f, 0xffff00ff },
                    {-1.0f, -1.0f, -1.0f, 0xffffff00 },
                    { 1.0f, -1.0f, -1.0f, 0xffffffff },
            };

    static const uint16_t s_cubeTriList[] =
            {
                    0, 1, 2, // 0
                    1, 3, 2,
                    4, 6, 5, // 2
                    5, 6, 7,
                    0, 2, 4, // 4
                    4, 2, 6,
                    1, 5, 3, // 6
                    5, 7, 3,
                    0, 4, 1, // 8
                    4, 5, 1,
                    2, 3, 6, // 10
                    6, 3, 7,
            };

    static const uint16_t s_cubeTriStrip[] =
            {
                    0, 1, 2,
                    3,
                    7,
                    1,
                    5,
                    0,
                    4,
                    2,
                    6,
                    7,
                    4,
                    5,
            };

    static const uint16_t s_cubeLineList[] =
            {
                    0, 1,
                    0, 2,
                    0, 4,
                    1, 3,
                    1, 5,
                    2, 3,
                    2, 6,
                    3, 7,
                    4, 5,
                    4, 6,
                    5, 7,
                    6, 7,
            };

    static const uint16_t s_cubeLineStrip[] =
            {
                    0, 2, 3, 1, 5, 7, 6, 4,
                    0, 2, 6, 4, 5, 7, 3, 1,
                    0,
            };

    static const uint16_t s_cubePoints[] =
            {
                    0, 1, 2, 3, 4, 5, 6, 7
            };

    static const char* s_ptNames[]
            {
                    "Triangle List",
                    "Triangle Strip",
                    "Lines",
                    "Line Strip",
                    "Points",
            };



    class Toc : public entry::AppI
    {
    public:
        Toc(const char* _name, const char* _description, const char* _url)
                : entry::AppI(_name, _description, _url)
                , m_pt(0)
                , m_r(true)
                , m_g(true)
                , m_b(true)
                , m_a(true)
        {
        }

        void init(int32_t _argc, const char* const* _argv, uint32_t _width, uint32_t _height) override
        {
            Args args(_argc, _argv);







            m_width  = _width;
            m_height = _height;
            m_debug  = BGFX_DEBUG_TEXT;
            m_reset  = BGFX_RESET_VSYNC;

            bgfx::Init init;
            init.type     = args.m_type;
            init.vendorId = args.m_pciId;
            init.resolution.width  = m_width;
            init.resolution.height = m_height;
            init.resolution.reset  = m_reset;
            bgfx::init(init);

            // Enable debug text.
            bgfx::setDebug(m_debug);

            // Set view 0 clear state.
            bgfx::setViewClear(0
                    , BGFX_CLEAR_COLOR|BGFX_CLEAR_DEPTH
                    , 0x303030ff
                    , 1.0f
                    , 0
            );






            // Create vertex stream declaration.
            PosColorVertex::init();

            // Create static vertex buffer.
            m_vbh = bgfx::createVertexBuffer(
                    // Static data can be passed with bgfx::makeRef
                    bgfx::makeRef(s_cubeVertices, sizeof(s_cubeVertices) )
                    , PosColorVertex::ms_layout
            );

            // Create static index buffer for triangle list rendering.
            m_ibh[0] = bgfx::createIndexBuffer(
                    // Static data can be passed with bgfx::makeRef
                    bgfx::makeRef(s_cubeTriList, sizeof(s_cubeTriList) )
            );

            // Create static index buffer for triangle strip rendering.
            m_ibh[1] = bgfx::createIndexBuffer(
                    // Static data can be passed with bgfx::makeRef
                    bgfx::makeRef(s_cubeTriStrip, sizeof(s_cubeTriStrip) )
            );

            // Create static index buffer for line list rendering.
            m_ibh[2] = bgfx::createIndexBuffer(
                    // Static data can be passed with bgfx::makeRef
                    bgfx::makeRef(s_cubeLineList, sizeof(s_cubeLineList) )
            );

            // Create static index buffer for line strip rendering.
            m_ibh[3] = bgfx::createIndexBuffer(
                    // Static data can be passed with bgfx::makeRef
                    bgfx::makeRef(s_cubeLineStrip, sizeof(s_cubeLineStrip) )
            );

            // Create static index buffer for point list rendering.
            m_ibh[4] = bgfx::createIndexBuffer(
                    // Static data can be passed with bgfx::makeRef
                    bgfx::makeRef(s_cubePoints, sizeof(s_cubePoints) )
            );

            // Create program from shaders.
             m_program = loadProgram("vs_cubes", "fs_cubes");
            m_timeOffset = bx::getHPCounter();
          //  imguiCreate();
        }

        virtual int shutdown() override
        {
           // imguiDestroy();

            // Shutdown bgfx.
            bgfx::shutdown();

            return 0;
        }

        bool update() override
        {
            if (!entry::processEvents(m_width, m_height, m_debug, m_reset, &m_mouseState) )
           {



               float time = (float)( (bx::getHPCounter()-m_timeOffset)/double(bx::getHPFrequency() ) );

               const bx::Vec3 at  = { 0.0f, 0.0f,   0.0f };
               const bx::Vec3 eye = { 0.0f, 0.0f, -35.0f };

               // Set view and projection matrix for view 0.
               {
                   float view[16];
                   bx::mtxLookAt(view, eye, at);

                   float proj[16];
                   bx::mtxProj(proj, 60.0f, float(m_width)/float(m_height), 0.1f, 100.0f, bgfx::getCaps()->homogeneousDepth);
                   bgfx::setViewTransform(0, view, proj);

                   // Set view 0 default viewport.
                   bgfx::setViewRect(0, 0, 0, uint16_t(m_width), uint16_t(m_height) );
               }

               // This dummy draw call is here to make sure that view 0 is cleared
               // if no other draw calls are submitted to view 0.
               bgfx::touch(0);

               bgfx::IndexBufferHandle ibh = m_ibh[m_pt];
               uint64_t state = 0
                                | (m_r ? BGFX_STATE_WRITE_R : 0)
                                | (m_g ? BGFX_STATE_WRITE_G : 0)
                                | (m_b ? BGFX_STATE_WRITE_B : 0)
                                | (m_a ? BGFX_STATE_WRITE_A : 0)
                                | BGFX_STATE_WRITE_Z
                                | BGFX_STATE_DEPTH_TEST_LESS
                                | BGFX_STATE_CULL_CW
                                | BGFX_STATE_MSAA
                                | s_ptState[m_pt]
               ;

               // Submit 11x11 cubes.
               for (uint32_t yy = 5; yy < 6; ++yy)
               {
                   for (uint32_t xx = 5; xx < 6; ++xx)
                   {
                       float mtx[16];

                 //      bx::mtxScale(mtx,3.0f,3,5);
                       bx::mtxRotateXY(mtx, time + xx*0.21f, time + yy*0.37f);



                       // Set model matrix for rendering.
                       bgfx::setTransform(mtx);



                       mtx[12] = -15.0f + float(xx)*3.0f;
                       mtx[13] = -15.0f + float(yy)*3.0f;
                       mtx[14] = 0.0f;
                       bgfx::setTransform(mtx);

                       // Set vertex and index buffer.
                       bgfx::setVertexBuffer(0, m_vbh);
                       bgfx::setIndexBuffer(ibh);

                       // Set render states.
                       bgfx::setState(state);

                       // Submit primitive for rendering to view 0.
                       bgfx::submit(0, m_program);
                   }
               }

               // Advance to next frame. Rendering thread will be kicked to
               // process submitted rendering primitives.
               bgfx::frame();
//
               return true;
          }

            return false;
        }

        entry::MouseState m_mouseState;

        uint32_t m_width;
        uint32_t m_height;
        uint32_t m_debug;
        uint32_t m_reset;
        bgfx::ProgramHandle m_program;
        int64_t m_timeOffset;

        bgfx::VertexBufferHandle m_vbh;
        bgfx::IndexBufferHandle m_ibh[BX_COUNTOF(s_ptState)];

        int32_t m_pt;

        bool m_r;
        bool m_g;
        bool m_b;
        bool m_a;
    };

} // namespace

#define MAX_WINDOWS 8

entry::WindowState m_windows[MAX_WINDOWS];
bgfx::FrameBufferHandle m_fbh[MAX_WINDOWS];
void createWindow()
{
    entry::WindowHandle handle = entry::createWindow(rand()%1280, rand()%1280, 600, 720);
    if (entry::isValid(handle) )
    {
        char str[256];
        bx::snprintf(str, BX_COUNTOF(str), "Window - handle %d", handle.idx);
        entry::setWindowTitle(handle, str);

        m_windows[handle.idx].m_handle = handle;
    }
}

void destroyWindow()
{
    for (uint32_t ii = 0; ii < MAX_WINDOWS; ++ii)
    {
        if (bgfx::isValid(m_fbh[ii]) )
        {
            bgfx::destroy(m_fbh[ii]);
            m_fbh[ii].idx = bgfx::kInvalidHandle;

            // Flush destruction of swap chain before destroying window!
            bgfx::frame();
            bgfx::frame();
        }

        if (entry::isValid(m_windows[ii].m_handle) )
        {
            entry::destroyWindow(m_windows[ii].m_handle);
            m_windows[ii].m_handle.idx = UINT16_MAX;
            return;
        }
    }
}


int _main_(int _argc, char** _argv)
{

    //initt

//   auto m_debug  = BGFX_DEBUG_TEXT;
//   auto m_reset  = BGFX_RESET_VSYNC;
//
//   bgfx::Init init;
//    init.type     =  bgfx::RendererType::Direct3D12;
//    init.vendorId = BGFX_PCI_ID_NONE;
//    init.resolution.width  = 400;
//    init.resolution.height = 400;
//    init.resolution.reset  = m_reset;
//
//    bgfx::init(init);
//
//    // Enable debug text.
//    bgfx::setDebug(m_debug);
//
//    // Set view 0 clear state.
//    bgfx::setViewClear(0
//            , BGFX_CLEAR_COLOR|BGFX_CLEAR_DEPTH
//            , 0x303030ff
//            , 1.0f
//            , 0
//    );
//    //initt

auto ar=Toc("tocc","tocc","tocc");
ar.init(_argc,_argv,400,400);
 entry::runApp(&ar,_argc,_argv);
//runApp(ar,_argc,_argv);
    return 0;
}


//ENTRY_IMPLEMENT_MAIN(
//        ExampleHelloWorld
//, "00-helloworld"
//, "Initialization and debug text."
//, "https://bkaradzic.github.io/bgfx/examples.html#helloworld"
//);
