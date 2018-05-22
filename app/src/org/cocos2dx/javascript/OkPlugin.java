package org.cocos2dx.javascript;

import org.json.JSONException;
import org.json.JSONArray;
import org.json.JSONObject;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager.NameNotFoundException;
import android.content.pm.PackageManager;
import android.content.pm.ResolveInfo;
import android.content.pm.Signature;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.os.AsyncTask;
import android.os.Handler;
import android.util.Base64;
import android.util.Log;
import android.widget.Toast;
import java.io.IOException;
import java.io.InputStream;
import java.net.HttpURLConnection;
import java.net.URL;
import java.util.EnumSet;
import java.util.HashMap;
import java.util.Map;

import ru.ok.android.sdk.OkRequestMode;
import ru.ok.android.sdk.Odnoklassniki;
import ru.ok.android.sdk.Shared;
import ru.ok.android.sdk.OkAuthActivity;
import ru.ok.android.sdk.OkListener;
import ru.ok.android.sdk.util.OkScope;
import ru.ok.android.sdk.util.OkDevice;
import ru.ok.android.sdk.util.OkAuthType;
import org.cocos2dx.lib.Cocos2dxHelper;
import ru.ok.android.sdk.OkAuthListener;
import android.preference.PreferenceManager.OnActivityResultListener;

public class OkPlugin {
    private static final String TAG = "OkPlugin";
    public static Activity appActivity;
    private static int loginCallbackId, viralCallbackId;
    private static Odnoklassniki odnoklassnikiObject;
    private static String REDIRECT_URL = "";
    private static String[] lastLoginPermissions = null;
    private static String mAppId;
    private static String mAppKey;
    private static final String ODKL_APP_SIGNATURE = "3082025b308201c4a00302010202044f6760f9300d06092a864886f70d01010505003071310c300a06035504061303727573310c300a06035504081303737062310c300a0603550407130373706231163014060355040a130d4f646e6f6b6c6173736e696b6931143012060355040b130b6d6f62696c65207465616d311730150603550403130e416e647265792041736c616d6f763020170d3132303331393136333831375a180f32303636313232313136333831375a3071310c300a06035504061303727573310c300a06035504081303737062310c300a0603550407130373706231163014060355040a130d4f646e6f6b6c6173736e696b6931143012060355040b130b6d6f62696c65207465616d311730150603550403130e416e647265792041736c616d6f7630819f300d06092a864886f70d010101050003818d003081890281810080bea15bf578b898805dfd26346b2fbb662889cd6aba3f8e53b5b27c43a984eeec9a5d21f6f11667d987b77653f4a9651e20b94ff10594f76a93a6a36e6a42f4d851847cf1da8d61825ce020b7020cd1bc2eb435b0d416908be9393516ca1976ff736733c1d48ff17cd57f21ad49e05fc99384273efc5546e4e53c5e9f391c430203010001300d06092a864886f70d0101050500038181007d884df69a9748eabbdcfe55f07360433b23606d3b9d4bca03109c3ffb80fccb7809dfcbfd5a466347f1daf036fbbf1521754c2d1d999f9cbc66b884561e8201459aa414677e411e66360c3840ca4727da77f6f042f2c011464e99f34ba7df8b4bceb4fa8231f1d346f4063f7ba0e887918775879e619786728a8078c76647ed";
    private static boolean inited = false;

	public static boolean init()    {
        if(!inited) {
            String appId = Util.getStringResByName("ok_app_id");
            String key = Util.getStringResByName("ok_api_key");
            Log.i(TAG, "OK initialize: " + appId + ", " + key);
            appActivity = Cocos2dxHelper.getActivity();
            REDIRECT_URL = "okauth://ok" + appId;
            mAppId = appId;
            mAppKey = key;
            odnoklassnikiObject = Odnoklassniki.createInstance(appActivity.getApplicationContext(), appId, key);
            Cocos2dxHelper.addOnActivityResultListener(new OnActivityResultListener() {
                    @Override
                    public boolean onActivityResult(int requestCode, int resultCode, Intent data) {
                        Log.i(TAG, "onActivityResult listener called");
                        OkPlugin.onActivityResult(requestCode, resultCode, data);
                        return true;
                    }
                });
            inited = true;
        }
        return true;
    }

    public static boolean login(final String[] permissions, final int callbackId)
    {
        init();
        lastLoginPermissions = permissions;
        odnoklassnikiObject.checkValidTokens(new OkListener() {
                @Override
                public void onSuccess(JSONObject json) {
                    //Log.i(TAG, "Token valid: "+json.toString());
                    final String token = json.optString("access_token");
                    final String uid = json.optString("logged_in_user");
                    final String sessionSecretKey = json.optString("session_secret_key");
                    Log.i(TAG, "Odnoklassniki accessToken = " + token);
					callLoginResult(callbackId, true, token);
                }
                @Override
                public void onError(String error) {
                    Log.e(TAG, "Token invalid. "+ error);
                    odnoklassnikiObject.clearTokens();
                    loginCallbackId = callbackId;
                    //вызываем запрос авторизации. После OAuth будет вызван callback, определенный для объекта
                    OkAuthType authType = OkAuthType.ANY;
                    if(permissions.length <= 0) {
                        String[] perm = new String[3];
                        perm[0] = OkScope.VALUABLE_ACCESS;
                        perm[1] = OkScope.LONG_ACCESS_TOKEN;
                        perm[3] = OkScope.APP_INVITE;
                        odnoklassnikiObject.requestAuthorization(appActivity, REDIRECT_URL, authType, perm);
                    } else {
                        odnoklassnikiObject.requestAuthorization(appActivity, REDIRECT_URL, authType, permissions);
                    }
                    Log.i(TAG, "Login requested with permissions:" + permissions.toString());
                }
            });
        return true;
    }

    public static boolean logout()
    {
        init();
        odnoklassnikiObject.clearTokens();
        return true;
    }

    public static boolean isLoggedIn(final int callbackId)
	{
		Log.e(TAG, "isLoggedIn");
        init();
        odnoklassnikiObject.checkValidTokens(new OkListener() {
                @Override
                public void onSuccess(JSONObject json) {
                    final String token = json.optString("access_token");
                    Log.i(TAG, "Odnoklassniki accessToken = " + token);
					callLoginResult(callbackId, true, token);
                }
                @Override
                public void onError(String error) {
					Log.e(TAG, "Token invalid. " + error);
					odnoklassnikiObject.clearTokens();
					callLoginResult(callbackId, false, error);
                }
            });
        return true;
	}

	private static void onActivityResult(int requestCode, int resultCode, Intent data) {
		Log.i(TAG, "onActivityResult:" + requestCode + "," + resultCode + "," + data);
		if (Odnoklassniki.getInstance().isActivityRequestOAuth(requestCode)) {
			Odnoklassniki.getInstance().onAuthActivityResult(requestCode, resultCode, data, getAuthListener());
		} else if (Odnoklassniki.getInstance().isActivityRequestViral(requestCode)) {
			Odnoklassniki.getInstance().onActivityResultResult(requestCode, resultCode, data, getRequestListener());
		}
	}

	private static OkAuthListener getAuthListener() {
		return new OkAuthListener() {
			@Override
			public void onSuccess(final JSONObject json) {
				//try {
					final String token = json.optString("access_token");
					Log.i(TAG, "Odnoklassniki accessToken = " + token);
					callLoginResult(loginCallbackId, true, token);
				/*} catch (JSONException e) {
					Log.i(TAG, e.toString());
					e.printStackTrace();
					callLoginResult(loginCallbackId, false, json.toString());
				}*/
			}

			@Override
			public void onError(String error) {
				callLoginResult(loginCallbackId, false, error);
			}

			@Override
			public void onCancel(String error) {
				callLoginResult(loginCallbackId, true, "");
			}
		};
	}
	private static OkListener getRequestListener() {
		return new OkListener() {
			@Override
			public void onSuccess(final JSONObject json) {
				Log.i(TAG, "Operation completed: " + json.toString());
				callRequestResult(viralCallbackId, null, json.toString());
				//Toast.makeText(MainActivity.this, json.toString(), Toast.LENGTH_LONG).show();
			}

			@Override
			public void onError(String error) {
				Log.e(TAG, "Posting error:" + error);
				callRequestResult(viralCallbackId, error, null);
				//Toast.makeText(MainActivity.this, String.format("%s: %s", getString(R.string.error), error), Toast.LENGTH_LONG).show();
			}
		};
	}
	/*
    public static void onActivityResult(int requestCode, int resultCode, Intent data)
    {
        if(resultCode == Activity.RESULT_CANCELED && data == null) {
            // switch to another activity result callback
            Log.i(TAG, "onActivityResult cancelled:" + requestCode + "," + resultCode + "," + data);
			if (loginCallbackId > 0) {
				callLoginResult(loginCallbackId, null);
				loginCallbackId = 0;
			} else if (viralCallbackId > 0) {
				callRequestResult(viralCallbackId, "cancelled", null);
				viralCallbackId = 0;
			}
            return;
        }
        Log.i(TAG, "onActivityResult:" + requestCode + "," + resultCode + "," + data);
        if (data != null && odnoklassnikiObject.isActivityRequestOAuth(requestCode)) {
            odnoklassnikiObject.onAuthActivityResult(requestCode, resultCode, data, new OkListener() {
                    @Override
                    public void onSuccess(final JSONObject json) {
                        final String token = json.optString("access_token");
                        final String uid = json.optString("logged_in_user");
                        final String sessionSecretKey = json.optString("session_secret_key");
                        Log.i(TAG, "Odnoklassniki accessToken = " + token);
						callLoginResult(loginCallbackId, token);
						loginCallbackId = 0;
                    }
                    @Override
                    public void onError(String error) {
                        Log.e(TAG, "OK login error: "+error);
						callLoginResult(loginCallbackId, null);
						loginCallbackId = 0;
                    }
                });
        } else if (data != null && odnoklassnikiObject.isActivityRequestViral(requestCode)) {
            odnoklassnikiObject.onActivityResultResult(requestCode, resultCode, data, new OkListener() {
                    @Override
                    public void onSuccess(final JSONObject json) {
                        Log.i(TAG, "Operation completed: "+json.toString());
						callRequestResult(viralCallbackId, null, json.toString());
						viralCallbackId = 0;
                    }
                    @Override
                    public void onError(String error) {
                        Log.e(TAG, "Posting error:"+error);
						callRequestResult(viralCallbackId, error, null);
						viralCallbackId = 0;
                    }
                });
        }
    }
  */
    static public boolean usersGetInfo(final String uids, final String fields, int callbackId) {
        init();
        Map<String, String> params = new HashMap<String, String>();
        if(uids != null && uids.length() > 0) {
            params.put("uids", uids);
            params.put("fields", fields);
            callApiMethod("users.getInfo", params, callbackId);
        } else {
            params.put("fields", fields);
            callApiMethod("users.getCurrentUser", params, callbackId);
        }
        return true;
    }

    static public boolean friendsGet(final String fid, final String sort_type, final int callbackId) {
        init();
        Map<String, String> params = new HashMap<String, String>();
        params.put("fid", fid);
        params.put("sort_type", sort_type);
        callApiMethod("friends.get", params, callbackId);
        return true;
    }

    static public boolean friendsGetOnline(final String uid, final String online, final int callbackId)
    {
        init();
        Map<String, String> params = new HashMap<String, String>();
        params.put("uid", uid);
        params.put("online", online);
        callApiMethod("friends.getOnline", params, callbackId);
        return true;
    }

    static public boolean share(final String url, final String comment, final int callbackId)
    {
        init();
        final Map<String, String> params = new HashMap<String, String>();
        params.put("linkUrl", url);
        params.put("comment", comment);
        callApiMethod("share.addLink", params, callbackId);
        return true;
    }

    static public boolean reportPayment(final String trx_id, final String amount, final String currency, final int callbackId)
    {
        init();
        Map<String, String> params = new HashMap<String, String>();
        params.put("trx_id", trx_id);
        params.put("amount", amount);
        params.put("currency", currency);
        callApiMethod("sdk.reportPayment", params, callbackId);
        return true;
    }

    static public boolean getInstallSource(final int callbackId)
    {
        init();
        Map<String, String> params = new HashMap<String, String>();
        params.put("adv_id", OkDevice.getAdvertisingId(appActivity.getApplicationContext()));
        getApiMethod("sdk.getInstallSource", params, callbackId);
        return true;
    }

    static public boolean isOkAppInstalled()
    {
        init();
        // check if OK application installed
        boolean ssoAvailable = false;
        final Intent intent = new Intent();
        intent.setClassName("ru.ok.android", "ru.ok.android.external.LoginExternal");
        final ResolveInfo resolveInfo = appActivity.getApplicationContext().getPackageManager().resolveActivity(intent, 0);
        if (resolveInfo != null) {
            try {
                final PackageInfo packageInfo = appActivity.getApplicationContext().getPackageManager().getPackageInfo(resolveInfo.activityInfo.packageName, PackageManager.GET_SIGNATURES);
                for (final Signature signature : packageInfo.signatures) {
                    if (signature.toCharsString().equals(ODKL_APP_SIGNATURE)) {
                        ssoAvailable = true;
                    }
                }
            } catch (NameNotFoundException exc) {
            }
        }
        if (ssoAvailable) {
            return true;
        } else {
            return false;
        }
    }

    static public boolean performPosting(String attachment, final int callbackId)
    {
        Log.w(TAG, "Posting attachment: "+attachment);
        init();
		odnoklassnikiObject.performPosting(appActivity, attachment, false, null);
        viralCallbackId = callbackId;
        return true;
    }

    static public boolean performSuggest(String arg, final int callbackId)
    {
        init();
        try {
            JSONObject json = new JSONObject(arg);
            HashMap<String, String> params = JsonHelper.toMap(json);
            odnoklassnikiObject.performAppSuggest(appActivity, params);
            viralCallbackId = callbackId;
            return true;
        } catch(JSONException ex) {
            Log.e(TAG, ex.toString());
            callRequestResult(callbackId, ex.toString(), null);
            return false;
        }
    }

    static public boolean reportStats(String arg, final int callbackId)
    {
        init();
        try {
            JSONObject json = new JSONObject(arg);
            HashMap<String, String> params = JsonHelper.toMap(json);
            getApiMethod("sdk.reportStats", params, callbackId);
            return true;
        } catch(JSONException ex) {
            Log.e(TAG, ex.toString());
            callRequestResult(callbackId, ex.toString(), null);
            return false;
        }
    }

    static public boolean performInvite(String arg, final int callbackId)
    {
        init();
        try {
            JSONObject json = new JSONObject(arg);
            HashMap<String, String> params = JsonHelper.toMap(json);
            odnoklassnikiObject.performAppInvite(appActivity, params);
            viralCallbackId = callbackId;
            return true;
        } catch(JSONException ex) {
            Log.e(TAG, ex.toString());
            callRequestResult(callbackId, ex.toString(), null);
            return false;
        }
    }

    static public boolean callApi(String method, String arg, final int callbackId)
    {
        init();
        try {
            JSONObject json = new JSONObject(arg);
            HashMap<String, String> params = JsonHelper.toMap(json);
            callApiMethod(method, params, callbackId);
            return true;
        } catch(JSONException ex) {
            Log.e(TAG, ex.toString());
            callRequestResult(callbackId, ex.toString(), null);
            return false;
        }
    }

    static public boolean getApi(String method, String arg, final int callbackId)
    {
        init();
        try {
            JSONObject json = new JSONObject(arg);
            HashMap<String, String> params = JsonHelper.toMap(json);
            getApiMethod(method, params, callbackId);
            return true;
        } catch(JSONException ex) {
            Log.e(TAG, ex.toString());
            callRequestResult(callbackId, ex.toString(), null);
            return false;
        }
    }

	static private void callApiMethod(final String method, final Map<String, String> params, final int callbackId) {
		Log.i(TAG, method);
		new AsyncTask<String, Void, String>() {
			@Override
			protected String doInBackground(String... args) {
				try {
					return odnoklassnikiObject.request(method, params, null);
                } catch (Exception e) {
                    Log.e(TAG, e.toString());
                    callRequestResult(callbackId, e.toString(), null);
                }
                return null;
            }
            @Override protected void onPostExecute(String result) {
                callRequestResult(callbackId, null, result);
            }
        }.execute();
    }

    static private void getApiMethod(final String method, final Map<String, String> params, final int callbackId) 
	{
		Log.i(TAG, method);
        new AsyncTask<String, Void, String>() {
            @Override protected String doInBackground(String... args) {
                try {
                    return odnoklassnikiObject.request(method, params, EnumSet.of(OkRequestMode.UNSIGNED));
                } catch (Exception e) {
                    Log.e(TAG, e.toString());
                    callRequestResult(callbackId, e.toString(), null);
                }
                return null;
            }
            @Override protected void onPostExecute(String result) {
                callRequestResult(callbackId, null, result);
            }
        }.execute();
    }

	static private void callLoginResult(final int callbackId, final boolean success, final String result) {
        appActivity.runOnUiThread(new Runnable() {
                @Override
                public void run() {
					Log.i(TAG, "callLoginResult " + (success ? "success" : "error") + " result: " + result);
					loginResult(callbackId, success, result);
                }
            });
    }

    static private void callRequestResult(final int callbackId, final String err, final String result) {
        appActivity.runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    Log.i(TAG, "OK request result: "+result);
                    requestResult(callbackId, err, result);
                }
            });
    }

	public static native void loginResult(int callbackId, boolean success, String result);
    public static native void requestResult(int callbackId, String err, String result);

}
