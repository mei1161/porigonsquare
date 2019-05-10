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
//  名前空間
//*****************************************************************************
using namespace DirectX;
using namespace SimpleMath;


//*****************************************************************************
//  定数
//*****************************************************************************
const int kWindowWidth = 1280; // ウィンドウサイズ（横）
const int kWindowHeight = 720;  // ウィンドウサイズ（縦）


//*****************************************************************************
//  プロトタイプ宣言
//*****************************************************************************
LRESULT CALLBACK windowProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam );


//*****************************************************************************
//  WinMain
//*****************************************************************************
int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow )
{
    // ウィンドウクラスの作成
    WNDCLASSEX wnd;
    ZeroMemory( &wnd, sizeof( wnd ) );                              // ゼロクリア
    wnd.cbSize = sizeof( WNDCLASSEX );                       // 構造体のサイズ
    wnd.style = CS_HREDRAW | CS_VREDRAW;                    // スタイル
    wnd.hInstance = hInstance;                                  // アプリケーションインスタンス
    wnd.lpszClassName = "ClassName";                                // クラス名
    wnd.hCursor = LoadCursor( NULL, IDC_ARROW );              // カーソル形状
    wnd.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1); // デフォルトの背景色
    wnd.lpfnWndProc = windowProc;                                 // コールバック関数ポインタの登録

    // ウィンドウクラスの登録
    if( !RegisterClassEx( &wnd ) )
    {
        // エラー
        return 0;
    }

    // ウィンドウスタイルの決定
    const DWORD style = WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX;
    const DWORD ex_style = WS_EX_OVERLAPPEDWINDOW;

    // クライアント領域が指定サイズになるように調整
    RECT rect = { 0L, 0L, kWindowWidth, kWindowHeight };
    AdjustWindowRectEx( &rect, style, false, ex_style );

    // ウィンドウの作成
    const HWND hWnd = CreateWindowEx(
        ex_style,               // 拡張ウィンドウスタイル
        "ClassName",            // クラス名
        "3DGame",               // ウィンドウ名
        style,                  // ウィンドウスタイル
        CW_USEDEFAULT,          // 表示座標X
        CW_USEDEFAULT,          // 表示座標Y
        rect.right - rect.left, // ウィンドウサイズX
        rect.bottom - rect.top, // ウィンドウサイズY
        NULL,                   // 親ウィンドウまたはオーナーウィンドウのハンドル
        NULL,                   // メニューハンドルまたは子識別子
        hInstance,              // アプリケーションのインスタンスハンドル
        NULL );                 // ウィンドウ作成データ

    // 機能レベルの設定
    D3D_FEATURE_LEVEL level_array[] =
    {
        D3D_FEATURE_LEVEL_11_0, // DirectX11
        D3D_FEATURE_LEVEL_10_1, // DirectX10.1
        D3D_FEATURE_LEVEL_10_0, // DirectX10.0
        D3D_FEATURE_LEVEL_9_3,  // DirectX9.3
    };

    // スワップチェインの設定
    DXGI_SWAP_CHAIN_DESC sc;
    ZeroMemory( &sc, sizeof( sc ) );                                        // 初期化
    sc.Windowed = true;                            // ウィンドウモード
    sc.OutputWindow = hWnd;                            // ウィンドウハンドル
    sc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT; // バックバッファの使用法
    sc.BufferCount = 1U;                              // バックバッファ数
    sc.BufferDesc.Width = kWindowWidth;                    // バックバッファサイズ（横）
    sc.BufferDesc.Height = kWindowHeight;                   // バックバッファサイズ（縦）
    sc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;      // フォーマット
    sc.BufferDesc.RefreshRate.Numerator = 60U;                             // リフレッシュレート分子
    sc.BufferDesc.RefreshRate.Denominator = 1U;                              // リフレッシュレート分母
    sc.SampleDesc.Count = 1;                               // マルチサンプリング
    sc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;	                     // モード自動切り替え

    // Direct3Dインターフェイス
    ID3D11Device*        d3d_device;     // Direct3Dデバイス
    ID3D11DeviceContext* device_context; // Direct3Dデバイスコンテキスト 
    IDXGISwapChain*      swap_chain;     // スワップチェイン
    D3D_FEATURE_LEVEL    feature_level;  // 機能レベル

    // 要素数を計算
    int num_feature_level = sizeof( level_array ) / sizeof( level_array[ 0 ] );

    // IDXGISwapChain、D3DDevice、D3DDeviceContextを作成する
    if( FAILED( D3D11CreateDeviceAndSwapChain(
        NULL,                      // DXGIアダプター(NULLにすると最初に見つけたディスプレイを描画先とする）
        D3D_DRIVER_TYPE_HARDWARE,  // ドライバータイプ
        NULL,                      // ソフトウェアラスタライザーを実装するDLLハンドル
        D3D11_CREATE_DEVICE_DEBUG, // フラグ
        level_array,               // 機能レベル
        num_feature_level,         // レベル個数
        D3D11_SDK_VERSION,         // 常にこの値
        &sc,                       // DXGI_SWAP_CHAIN_DESC構造体のポインタを渡す
        &swap_chain,               // 受取先IDXGISwapChainポインタの変数を指定
        &d3d_device,               // 受取先ID3D11Deviceポインタの変数を指定
        &feature_level,            // 設定した機能レベル
        &device_context ) ) )      // 受取先ID3D11DeviceContextポインタの変数を指定
    {
        // エラー
        return 0;
    }

    // バックバッファ描画ターゲットの取得
    ID3D11Texture2D* backbuffer = NULL;
    if( FAILED( swap_chain->GetBuffer( 0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backbuffer) ) ) )
    {
        // エラー
        return 0;
    }

    // 描画ターゲット・ビューの作成
    ID3D11RenderTargetView* render_target_view = NULL;
    if( FAILED( d3d_device->CreateRenderTargetView(
        backbuffer,               // ビューでアクセスするリソース
        NULL,                     // ターゲットビューの定義
        &render_target_view ) ) ) // ターゲットビュー格納先
    {
        // エラー
        return 0;
    }

    // バックバッファ解放(必要なことは伝えてもう使用することはないのでここで解放）
    backbuffer->Release();

    // 描画ターゲットビューを出力マネージャーの描画ターゲットとして設定
    device_context->OMSetRenderTargets(
        1,					 // 描画ターゲット数（8まで可)
        &render_target_view, // ターゲットビュー配列
        NULL );              // 深度/ステンシルビュー

    // ビューポートの作成と設定
    D3D11_VIEWPORT vp;
    ZeroMemory( &vp, sizeof( vp ) );
    vp.Width = static_cast<float>(kWindowWidth);  // 描画領域幅
    vp.Height = static_cast<float>(kWindowHeight); // 描画領域高さ
    vp.MinDepth = 0.0F;  // 描画領域最小深度値
    vp.MaxDepth = 1.0F;  // 描画領域最大深度値
    device_context->RSSetViewports( 1, &vp );

    // COMの初期化（これを行わないとテクスチャの読み込みに失敗する）
    if( FAILED( CoInitializeEx( NULL, COINIT_MULTITHREADED ) ) )
    {
        // エラー
        return 0;
    }


    //プリミティブバッチクラスの変数宣言
    PrimitiveBatch<VertexPositionColor> primitive( device_context );


    //エフェクトクラスの変数宣言
    BasicEffect effect( d3d_device );

    //頂点カラーを有効にする（このフラグを設定しないと色情報を持っていても出力されない)
    effect.SetVertexColorEnabled( true );

    //コンパイル済みシェーダへのポインタとサイズを取得
    const void* blob;
    size_t size;
    effect.GetVertexShaderBytecode( &blob, &size );

    //頂点レイアウトの作成
    ID3D11InputLayout* layout;

    if( FAILED( d3d_device->CreateInputLayout(
        VertexPositionColor::InputElements,
        VertexPositionColor::InputElementCount,
        blob,
        size,
        &layout ) ) )
    {
        // エラー
        return 0;
    }

    //頂点データの作成
    //Vector3…座標　Vector4…色情報
    VertexPositionColor v0( Vector3( -0.5F, 0.5F, 0.0F ), Vector4( 1.0F, 0.0F, 0.0F, 1.0F ) );
    VertexPositionColor v1( Vector3( 0.5F, -0.5F, 0.0F ), Vector4( 0.0F, 1.0F, 0.0F, 1.0F ) );
    VertexPositionColor v2( Vector3( -0.5F, -0.5F, 0.0F ), Vector4( 0.0F, 0.0F, 1.0F, 1.0F ) );

    //配列の作成
    VertexPositionColor p[] = { v0,v1,v2 };


    // ウィンドウの表示
    ShowWindow( hWnd, SW_SHOWNORMAL );

    // メインループ
    MSG msg = { NULL };

    // 時間計測
    DWORD t1, t2, t3 = 0L, dt;
    t1 = timeGetTime();
    t2 = timeGetTime();

    while( msg.message != WM_QUIT )
    {
        // メッセージ処理
        if( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) )
        {
            TranslateMessage( &msg );
            DispatchMessage( &msg );
        }
        else
        {
            // フレームレート制御
            t1 = timeGetTime();   // 現在の時間
            dt = (t1 - t2) + t3;  // 前回の更新からの差分を計算

            // 約16ミリ秒以上経過していたら更新する
            if( dt > 16 )
            {
                t2 = t1;      // 今の時間を前回の時間とする
                t3 = dt % 16; // 誤差分を吸収

                //エフェクトの設定を適用
                effect.Apply( device_context );

                //頂点レイアウトの設定
                device_context->IASetInputLayout( layout );

                // 画面クリア（指定したカラーで塗りつぶす）
                device_context->ClearRenderTargetView( render_target_view, Colors::CornflowerBlue );

                //プリミティブの描画開始
                primitive.Begin();

                //描画
                // 第一引数：三角形の描画方法
                // 第二引数：描画する頂点配列の先頭ポインタ
                // 第三引数：頂点数
                primitive.Draw( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST, p, 3 );

                //描画終了
                primitive.End();
                // 画面更新処理(フロントバッファとバックバッファを入れ替える）
                swap_chain->Present( 1, 0 );
            }
        }
    }

    // COMライブラリの解放
    CoUninitialize();

    // ゲームを終了する前にインターフェイスの解放を行う（確保した順の逆に解放していく）
    layout->Release();
    render_target_view->Release(); // 描画ターゲット
    swap_chain->Release();         // スワップチェイン
    device_context->ClearState();  // 既定の設定に戻す
    device_context->Release();     // デバイスコンテキスト
    d3d_device->Release();         // D3Dデバイス

    return 0;
}


//*****************************************************************************
//  ウィンドウプロシージャ
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
        case VK_ESCAPE: // ESCキーが押されたら終了
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