//*****************************************************************************
//  Copyright (C) 2019 KOUDO All Rights Reserved.
//*****************************************************************************
#include <chrono>
#include <Windows.h>
#include <d3d11.h>
#include <DirectXColors.h>
#include <SimpleMath.h>


#include<PrimitiveBatch.h>
#include<VertexTypes.h>
#include<Effects.h>
//*****************************************************************************
//  ���O���
//*****************************************************************************
using namespace DirectX;
using namespace SimpleMath;


//*****************************************************************************
//  �萔
//*****************************************************************************
const int kWindowWidth = 1280; // �E�B���h�E�T�C�Y�i���j
const int kWindowHeight = 720;  // �E�B���h�E�T�C�Y�i�c�j


//*****************************************************************************
//  �v���g�^�C�v�錾
//*****************************************************************************
LRESULT CALLBACK windowProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam );


//*****************************************************************************
//  WinMain
//*****************************************************************************
int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow )
{
    // �E�B���h�E�N���X�̍쐬
    WNDCLASSEX wnd;
    ZeroMemory( &wnd, sizeof( wnd ) );                              // �[���N���A
    wnd.cbSize = sizeof( WNDCLASSEX );                       // �\���̂̃T�C�Y
    wnd.style = CS_HREDRAW | CS_VREDRAW;                    // �X�^�C��
    wnd.hInstance = hInstance;                                  // �A�v���P�[�V�����C���X�^���X
    wnd.lpszClassName = "ClassName";                                // �N���X��
    wnd.hCursor = LoadCursor( NULL, IDC_ARROW );              // �J�[�\���`��
    wnd.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1); // �f�t�H���g�̔w�i�F
    wnd.lpfnWndProc = windowProc;                                 // �R�[���o�b�N�֐��|�C���^�̓o�^

    // �E�B���h�E�N���X�̓o�^
    if( !RegisterClassEx( &wnd ) )
    {
        // �G���[
        return 0;
    }

    // �E�B���h�E�X�^�C���̌���
    const DWORD style = WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX;
    const DWORD ex_style = WS_EX_OVERLAPPEDWINDOW;

    // �N���C�A���g�̈悪�w��T�C�Y�ɂȂ�悤�ɒ���
    RECT rect = { 0L, 0L, kWindowWidth, kWindowHeight };
    AdjustWindowRectEx( &rect, style, false, ex_style );

    // �E�B���h�E�̍쐬
    const HWND hWnd = CreateWindowEx(
        ex_style,               // �g���E�B���h�E�X�^�C��
        "ClassName",            // �N���X��
        "3DGame",               // �E�B���h�E��
        style,                  // �E�B���h�E�X�^�C��
        CW_USEDEFAULT,          // �\�����WX
        CW_USEDEFAULT,          // �\�����WY
        rect.right - rect.left, // �E�B���h�E�T�C�YX
        rect.bottom - rect.top, // �E�B���h�E�T�C�YY
        NULL,                   // �e�E�B���h�E�܂��̓I�[�i�[�E�B���h�E�̃n���h��
        NULL,                   // ���j���[�n���h���܂��͎q���ʎq
        hInstance,              // �A�v���P�[�V�����̃C���X�^���X�n���h��
        NULL );                 // �E�B���h�E�쐬�f�[�^

    // �@�\���x���̐ݒ�
    D3D_FEATURE_LEVEL level_array[] =
    {
        D3D_FEATURE_LEVEL_11_0, // DirectX11
        D3D_FEATURE_LEVEL_10_1, // DirectX10.1
        D3D_FEATURE_LEVEL_10_0, // DirectX10.0
        D3D_FEATURE_LEVEL_9_3,  // DirectX9.3
    };

    // �X���b�v�`�F�C���̐ݒ�
    DXGI_SWAP_CHAIN_DESC sc;
    ZeroMemory( &sc, sizeof( sc ) );                                        // ������
    sc.Windowed = true;                            // �E�B���h�E���[�h
    sc.OutputWindow = hWnd;                            // �E�B���h�E�n���h��
    sc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT; // �o�b�N�o�b�t�@�̎g�p�@
    sc.BufferCount = 1U;                              // �o�b�N�o�b�t�@��
    sc.BufferDesc.Width = kWindowWidth;                    // �o�b�N�o�b�t�@�T�C�Y�i���j
    sc.BufferDesc.Height = kWindowHeight;                   // �o�b�N�o�b�t�@�T�C�Y�i�c�j
    sc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;      // �t�H�[�}�b�g
    sc.BufferDesc.RefreshRate.Numerator = 60U;                             // ���t���b�V�����[�g���q
    sc.BufferDesc.RefreshRate.Denominator = 1U;                              // ���t���b�V�����[�g����
    sc.SampleDesc.Count = 1;                               // �}���`�T���v�����O
    sc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;	                     // ���[�h�����؂�ւ�

    // Direct3D�C���^�[�t�F�C�X
    ID3D11Device*        d3d_device;     // Direct3D�f�o�C�X
    ID3D11DeviceContext* device_context; // Direct3D�f�o�C�X�R���e�L�X�g 
    IDXGISwapChain*      swap_chain;     // �X���b�v�`�F�C��
    D3D_FEATURE_LEVEL    feature_level;  // �@�\���x��

    // �v�f�����v�Z
    int num_feature_level = sizeof( level_array ) / sizeof( level_array[ 0 ] );

    // IDXGISwapChain�AD3DDevice�AD3DDeviceContext���쐬����
    if( FAILED( D3D11CreateDeviceAndSwapChain(
        NULL,                      // DXGI�A�_�v�^�[(NULL�ɂ���ƍŏ��Ɍ������f�B�X�v���C��`���Ƃ���j
        D3D_DRIVER_TYPE_HARDWARE,  // �h���C�o�[�^�C�v
        NULL,                      // �\�t�g�E�F�A���X�^���C�U�[����������DLL�n���h��
        D3D11_CREATE_DEVICE_DEBUG, // �t���O
        level_array,               // �@�\���x��
        num_feature_level,         // ���x����
        D3D11_SDK_VERSION,         // ��ɂ��̒l
        &sc,                       // DXGI_SWAP_CHAIN_DESC�\���̂̃|�C���^��n��
        &swap_chain,               // ����IDXGISwapChain�|�C���^�̕ϐ����w��
        &d3d_device,               // ����ID3D11Device�|�C���^�̕ϐ����w��
        &feature_level,            // �ݒ肵���@�\���x��
        &device_context ) ) )      // ����ID3D11DeviceContext�|�C���^�̕ϐ����w��
    {
        // �G���[
        return 0;
    }

    // �o�b�N�o�b�t�@�`��^�[�Q�b�g�̎擾
    ID3D11Texture2D* backbuffer = NULL;
    if( FAILED( swap_chain->GetBuffer( 0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backbuffer) ) ) )
    {
        // �G���[
        return 0;
    }

    // �`��^�[�Q�b�g�E�r���[�̍쐬
    ID3D11RenderTargetView* render_target_view = NULL;
    if( FAILED( d3d_device->CreateRenderTargetView(
        backbuffer,               // �r���[�ŃA�N�Z�X���郊�\�[�X
        NULL,                     // �^�[�Q�b�g�r���[�̒�`
        &render_target_view ) ) ) // �^�[�Q�b�g�r���[�i�[��
    {
        // �G���[
        return 0;
    }

    // �o�b�N�o�b�t�@���(�K�v�Ȃ��Ƃ͓`���Ă����g�p���邱�Ƃ͂Ȃ��̂ł����ŉ���j
    backbuffer->Release();

    // �`��^�[�Q�b�g�r���[���o�̓}�l�[�W���[�̕`��^�[�Q�b�g�Ƃ��Đݒ�
    device_context->OMSetRenderTargets(
        1,					 // �`��^�[�Q�b�g���i8�܂ŉ�)
        &render_target_view, // �^�[�Q�b�g�r���[�z��
        NULL );              // �[�x/�X�e���V���r���[

    // �r���[�|�[�g�̍쐬�Ɛݒ�
    D3D11_VIEWPORT vp;
    ZeroMemory( &vp, sizeof( vp ) );
    vp.Width = static_cast<float>(kWindowWidth);  // �`��̈敝
    vp.Height = static_cast<float>(kWindowHeight); // �`��̈捂��
    vp.MinDepth = 0.0F;  // �`��̈�ŏ��[�x�l
    vp.MaxDepth = 1.0F;  // �`��̈�ő�[�x�l
    device_context->RSSetViewports( 1, &vp );

    // COM�̏������i������s��Ȃ��ƃe�N�X�`���̓ǂݍ��݂Ɏ��s����j
    if( FAILED( CoInitializeEx( NULL, COINIT_MULTITHREADED ) ) )
    {
        // �G���[
        return 0;
    }


    //�v���~�e�B�u�o�b�`�N���X�̕ϐ��錾
    PrimitiveBatch<VertexPositionColor> primitive( device_context );


    //�G�t�F�N�g�N���X�̕ϐ��錾
    BasicEffect effect( d3d_device );

    //���_�J���[��L���ɂ���i���̃t���O��ݒ肵�Ȃ��ƐF���������Ă��Ă��o�͂���Ȃ�)
    effect.SetVertexColorEnabled( true );

    //�R���p�C���ς݃V�F�[�_�ւ̃|�C���^�ƃT�C�Y���擾
    const void* blob;
    size_t size;
    effect.GetVertexShaderBytecode( &blob, &size );

    //���_���C�A�E�g�̍쐬
    ID3D11InputLayout* layout;

    if( FAILED( d3d_device->CreateInputLayout(
        VertexPositionColor::InputElements,
        VertexPositionColor::InputElementCount,
        blob,
        size,
        &layout ) ) )
    {
        // �G���[
        return 0;
    }

    //���_�f�[�^�̍쐬
    //Vector3�c���W�@Vector4�c�F���
    VertexPositionColor v0( Vector3( -0.5F, 0.5F, 0.0F ), Vector4( 1.0F, 0.0F, 0.0F, 1.0F ) );
    VertexPositionColor v1( Vector3( 0.5F, -0.5F, 0.0F ), Vector4( 0.0F, 1.0F, 0.0F, 1.0F ) );
    VertexPositionColor v2( Vector3( -0.5F, -0.5F, 0.0F ), Vector4( 0.0F, 0.0F, 1.0F, 1.0F ) );

    //�z��̍쐬
    VertexPositionColor p[] = { v0,v1,v2 };


    // �E�B���h�E�̕\��
    ShowWindow( hWnd, SW_SHOWNORMAL );

    // ���C�����[�v
    MSG msg = { NULL };

    // ���Ԍv��
    DWORD t1, t2, t3 = 0L, dt;
    t1 = timeGetTime();
    t2 = timeGetTime();

    while( msg.message != WM_QUIT )
    {
        // ���b�Z�[�W����
        if( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) )
        {
            TranslateMessage( &msg );
            DispatchMessage( &msg );
        }
        else
        {
            // �t���[�����[�g����
            t1 = timeGetTime();   // ���݂̎���
            dt = (t1 - t2) + t3;  // �O��̍X�V����̍������v�Z

            // ��16�~���b�ȏ�o�߂��Ă�����X�V����
            if( dt > 16 )
            {
                t2 = t1;      // ���̎��Ԃ�O��̎��ԂƂ���
                t3 = dt % 16; // �덷�����z��

                //�G�t�F�N�g�̐ݒ��K�p
                effect.Apply( device_context );

                //���_���C�A�E�g�̐ݒ�
                device_context->IASetInputLayout( layout );

                // ��ʃN���A�i�w�肵���J���[�œh��Ԃ��j
                device_context->ClearRenderTargetView( render_target_view, Colors::CornflowerBlue );

                //�v���~�e�B�u�̕`��J�n
                primitive.Begin();

                //�`��
                // �������F�O�p�`�̕`����@
                // �������F�`�悷�钸�_�z��̐擪�|�C���^
                // ��O�����F���_��
                primitive.Draw( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST, p, 3 );

                //�`��I��
                primitive.End();
                // ��ʍX�V����(�t�����g�o�b�t�@�ƃo�b�N�o�b�t�@�����ւ���j
                swap_chain->Present( 1, 0 );
            }
        }
    }

    // COM���C�u�����̉��
    CoUninitialize();

    // �Q�[�����I������O�ɃC���^�[�t�F�C�X�̉�����s���i�m�ۂ������̋t�ɉ�����Ă����j
    layout->Release();
    render_target_view->Release(); // �`��^�[�Q�b�g
    swap_chain->Release();         // �X���b�v�`�F�C��
    device_context->ClearState();  // ����̐ݒ�ɖ߂�
    device_context->Release();     // �f�o�C�X�R���e�L�X�g
    d3d_device->Release();         // D3D�f�o�C�X

    return 0;
}


//*****************************************************************************
//  �E�B���h�E�v���V�[�W��
//*****************************************************************************
LRESULT CALLBACK windowProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
    PAINTSTRUCT ps;
    HDC hdc;

    switch( msg )
    {
    case WM_PAINT:
        hdc = BeginPaint( hWnd, &ps );
        EndPaint( hWnd, &ps );
        break;

    case WM_KEYDOWN:
        switch( wParam )
        {
        case VK_ESCAPE: // ESC�L�[�������ꂽ��I��
            PostMessage( hWnd, WM_CLOSE, 0, 0 );
            break;
        }
        break;

    case WM_DESTROY:
        PostQuitMessage( 0 );
        break;
    }

    return DefWindowProc( hWnd, msg, wParam, lParam );
}