package ca.bcit.voicegame;

import androidx.annotation.NonNull;
import androidx.appcompat.app.AppCompatActivity;

import android.graphics.Bitmap;
import android.os.Bundle;
import android.webkit.JavascriptInterface;
import android.webkit.ValueCallback;
import android.webkit.WebResourceError;
import android.webkit.WebResourceRequest;
import android.webkit.WebResourceResponse;
import android.webkit.WebSettings;
import android.webkit.WebView;
import android.webkit.WebViewClient;

public class TttActivity
        extends AppCompatActivity {
    private WebView webView;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        WebSettings settings;

        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_ttt);

        webView = (WebView) findViewById(R.id.webView);

        settings = webView.getSettings();
        settings.setJavaScriptEnabled(true);
        settings.setBuiltInZoomControls(true);
        settings.setDisplayZoomControls(false);
        settings.setCacheMode(WebSettings.LOAD_NO_CACHE);

        final String htmlString =
                "<!DOCTYPE html>" +
                        "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">" +
                        "<html>" +
                        "<head>" +
                        "<title>Title of the document</title>" +
                        "</head>" +
                        "<body>" +
                        "<button onclick=\"pressed()\">Click me!</button>" +
                        "<script>" +
                        "function pressed()" +
                        "{" +
                        "let handled = false;" +
                        "try" +
                        "{" +
                        "if(typeof Android !== \"undefined\" && Android !== null)" +
                        "{" +
                        "Android[\"sayHello\"]('Hello, World 2!');" +
                        "handled = true;" +
                        "}" +
                        "else if(typeof webkit !== \"undefined\" &&" +
                        "typeof webkit !== null &&" +
                        "typeof webkit.messageHandlers !== \"undefined\" &&" +
                        "typeof webkit.messageHandlers !== null &&" +
                        "typeof webkit.messageHandlers[\"sayHello\"] !== \"undefined\" &&" +
                        "typeof webkit.messageHandlers[\"sayHello\"] !== null)" +
                        "{" +
                        "webkit.messageHandlers[\"sayHello\"].postMessage('Hello, World 2!');" +
                        "handled = true;" +
                        "}" +
                        "}" +
                        "catch(error)" +
                        "{" +
                        "throw error;" +
                        "}" +
                        "if(!(handled))" +
                        "{" +
                        "throw \"Unable to handle \" + app.messageHandlerName;" +
                        "}" +
                        "}" +
                        "</script>" +
                        "<script>" +
                        "function fromNative(colour)" +
                        "{" +
                        "document.body.style.background = colour;" +
                        "}" +
                        "</script>" +
                        "</body>" +
                        "</html>";

        webView.addJavascriptInterface(new WebAppInterface(), "Android");
        webView.loadData(htmlString, "text/html; charset=utf-8", "UTF8");
    }

    private class WebAppInterface
    {
        private final String TAG = WebAppInterface.class.getName();

        @JavascriptInterface
        public void sayHello(final @NonNull String msg)
        {
            System.out.println(String.format("Hello with message: \"%s\"", msg));

            webView.post(new Runnable() {
                @Override
                public void run() {
                    webView.evaluateJavascript("fromNative('red')", new ValueCallback<String>() {
                        @Override
                        public void onReceiveValue(String value) {
                            System.out.println("Done");
                        }
                    });
                }
            });
        }
    }
}
