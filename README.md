# Description
This plugin adds Odnoklassniki integration for sdkbar plugin system.

# Installation

`sdkbar -i https://github.com/OrangeAppsRu/sdkbar-ok`

# Dependencies

This plugins depends on `sdkbar-utils` (https://github.com/OrangeAppsRu/sdkbar-utils).

# Plugin JS interface

- `oksdk.init(app_id, app_key)` :deprecated
- `oksdk.login(permissions_array, callback_function, callback_this)`
- `oksdk.logout()`
- `oksdk.loggedin(callback_function, callback_this)`
- `oksdk.users_get(uids, fields, callback_function, callback_this)`
- `oksdk.friends_get(friend_id, sort_type, callback_function, callback_this)`
- `oksdk.friends_get_online(uid, online, callback_function, callback_this)`
- `oksdk.share(url, comment, callback_function, callback_this)`
- `oksdk.report_payment(trx_id, amount, currency, callback_function, callback_this)`
- `oksdk.get_install_source(callback_function, callback_this)`
- `oksdk.is_ok_app_installed()` returns bool
- `oksdk.perform_posting(attachment, callback_function, callback_this)` 
- `oksdk.perform_suggest(params_dictionary, callback_function, callback_this)`
- `oksdk.report_stats(params_dictionary, callback_function, callback_this)`
- `oksdk.perform_invite(params_dictionary, callback_function, callback_this)`
- `oksdk.call_api(method, params_dictionary, callback_function, callback_this)` make signed API request
- `oksdk.get_api(method, params_dictionary, callback_function, callback_this)` make unsigned API request
