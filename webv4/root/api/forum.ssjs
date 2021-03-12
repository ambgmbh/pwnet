/*  This script is an interface between HTTP clients and the functions defined
    in web/lib/forum.js.  A basic check for an authenticated, non-guest user
    is done here; otherwise all permission checking is done at the function
    level. */

var settings = load('modopts.js', 'web') || { web_directory: '../webv4' };
load(settings.web_directory + '/lib/init.js');

load(settings.web_lib + 'auth.js');
var forum = require({}, settings.web_lib + 'forum.js', 'forum');
var request = require({}, settings.web_lib + 'request.js', 'request');

var reply = {};
var replied = false;

// Remember to test the 'mail' page afterward too, including block-sender

// There must be an API call, and the user must not be a guest or unknown
if (request.hasParam('call') && (http_request.method === 'GET' || http_request.method === 'POST')) {

    var handled = false;

    // Authenticated calls
    if (user.number > 0 && user.alias !== settings.guest) {

        handled = true;

        switch (http_request.query.call[0].toLowerCase()) {

            // case 'block-sender':
            //     if (user.is_sysop) {
            //         if (http_request.query.from) addTwit(decodeURIComponent(http_request.query.from[0]));
            //         if (http_request.query.from_net) addTwit(decodeURIComponent(http_request.query.from_net[0]));
            //         reply.err = null;
            //     }
            //     break;

            case 'get-signature':
                reply = forum.getSignature(user.number);
                break;

            default:
                handled = false;
                break;

        }

    }

    // Unauthenticated calls
    if (!handled) {

        switch (request.getParam('call').toLowerCase()) {

            case 'get-thread':
                if (request.hasParams(['sub', 'thread'])) {
                    http_reply.header['Content-Type'] = 'text/plain; charset=utf8';
                    forum.getThread(request.getParam('sub'), request.getParam('thread'), function (m) {
                        writeln(JSON.stringify(m));
                    });
                    replied = true;
                }
                break;

            case 'list-groups':
                reply = forum.listGroups();
                break;

            case 'list-subs':
                if (request.hasParam('group')) reply = forum.listSubs(request.getParam('group'));
                break;

            case 'list-threads':
                if (request.hasParam('sub')) reply = forum.listThreads(request.getParam('sub'));
                break;

            case 'get-newest-message-per-sub':
                if (request.hasParam('group')) reply = forum.getNewestMessagePerSub(request.getParam('group'));
                break;

            case 'get-thread-list':
                if (request.hasParam('sub')) reply = forum.getThreadList(request.getParam('sub'));
                break;

            default:
                break;

        }

    }

}

if (!replied) {
    reply = JSON.stringify(reply);
    http_reply.header['Content-Type'] = 'application/json';
    http_reply.header['Content-Length'] = reply.length;
    write(reply);
}

reply = undefined;