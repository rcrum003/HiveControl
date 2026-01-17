<?php
/**
 * Security Initialization
 * Configures secure session settings and CSRF protection
 * Version 1.0 - 2026-01-17
 */

// SECURITY: Configure secure session settings
ini_set('session.cookie_httponly', 1);  // Prevent JavaScript access to session cookie
// ini_set('session.cookie_secure', 1);     // Require HTTPS (DISABLED - not using HTTPS)
ini_set('session.cookie_samesite', 'Strict');  // Prevent CSRF attacks
ini_set('session.use_strict_mode', 1);   // Reject uninitialized session IDs
ini_set('session.gc_maxlifetime', 3600); // 1 hour session timeout

// Start session if not already started
if (session_status() == PHP_SESSION_NONE) {
    session_start();
}

// Regenerate session ID periodically to prevent session fixation
if (!isset($_SESSION['last_regeneration'])) {
    $_SESSION['last_regeneration'] = time();
} elseif (time() - $_SESSION['last_regeneration'] > 300) {  // Every 5 minutes
    session_regenerate_id(true);
    $_SESSION['last_regeneration'] = time();
}

// Generate CSRF token if not exists
if (empty($_SESSION['csrf_token'])) {
    $_SESSION['csrf_token'] = bin2hex(random_bytes(32));
}

/**
 * Get CSRF token for forms
 * @return string CSRF token
 */
function get_csrf_token() {
    return $_SESSION['csrf_token'] ?? '';
}

/**
 * Generate CSRF token hidden input field
 * @return string HTML input field
 */
function csrf_field() {
    $token = htmlspecialchars(get_csrf_token());
    return '<input type="hidden" name="csrf_token" value="' . $token . '">';
}

/**
 * Verify CSRF token from POST request
 * @return bool True if valid, false otherwise
 */
function verify_csrf_token() {
    $token = $_POST['csrf_token'] ?? '';
    $session_token = $_SESSION['csrf_token'] ?? '';

    if (empty($token) || empty($session_token)) {
        return false;
    }

    return hash_equals($session_token, $token);
}

/**
 * Check CSRF token and die if invalid
 */
function require_csrf_token() {
    if (!verify_csrf_token()) {
        error_log("CSRF token validation failed from IP " . $_SERVER['REMOTE_ADDR']);
        die('CSRF token validation failed. Please refresh the page and try again.');
    }
}

// SECURITY: Add security headers
header("X-Frame-Options: DENY");
header("X-Content-Type-Options: nosniff");
header("Referrer-Policy: strict-origin-when-cross-origin");
// CSP header - adjust as needed for your application
header("Content-Security-Policy: default-src 'self'; script-src 'self' 'unsafe-inline'; style-src 'self' 'unsafe-inline'; img-src 'self' data:;");

?>
