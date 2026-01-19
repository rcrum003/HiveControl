<h1><i class="fa fa-lock"></i> Change Password</h1>

<div class="alert alert-info">
    <strong>Security First:</strong> Regular password changes and strong passwords help protect your HiveControl system from unauthorized access.
</div>

<h2>Why Change Your Password?</h2>

<p>Regular password updates protect your system by:</p>
<ul>
    <li><strong>Security Maintenance:</strong> Reduce risk from compromised credentials</li>
    <li><strong>Access Control:</strong> Revoke access when credentials may have been shared</li>
    <li><strong>Best Practice:</strong> Follow security recommendations for sensitive systems</li>
    <li><strong>Compliance:</strong> Meet organizational security policies</li>
</ul>

<h2>Changing Your Password</h2>

<div class="panel panel-default">
    <div class="panel-heading">
        <strong>Password Change Process</strong>
    </div>
    <div class="panel-body">
        <p><span class="step-number">1</span> Navigate to <strong>Settings > Change Password</strong></p>
        <p><span class="step-number">2</span> Enter your <strong>Current Password</strong></p>
        <p><span class="step-number">3</span> Enter your <strong>New Password</strong></p>
        <p><span class="step-number">4</span> <strong>Confirm</strong> your new password by entering it again</p>
        <p><span class="step-number">5</span> Click <strong>"Change Password"</strong></p>
        <p><span class="step-number">6</span> You'll be logged out automatically</p>
        <p><span class="step-number">7</span> Log back in with your new password</p>
    </div>
</div>

<h2>Password Requirements</h2>

<div class="feature-box">
    <h4><i class="fa fa-shield"></i> Strong Password Guidelines</h4>
    <p>Your password must meet these minimum requirements:</p>
    <ul>
        <li><strong>Length:</strong> At least 8 characters (12+ recommended)</li>
        <li><strong>Complexity:</strong> Mix of uppercase, lowercase, numbers, and symbols</li>
        <li><strong>Uniqueness:</strong> Different from previous passwords</li>
        <li><strong>Non-obvious:</strong> Not based on dictionary words or personal info</li>
    </ul>

    <div class="alert alert-warning">
        <strong>Avoid Common Mistakes:</strong>
        <ul style="margin-bottom: 0;">
            <li>Don't use "password", "123456", or similar common passwords</li>
            <li>Don't use your username, email, or hive name</li>
            <li>Don't use sequential patterns (abc123, qwerty)</li>
            <li>Don't reuse passwords from other sites</li>
        </ul>
    </div>
</div>

<h2>Creating Strong Passwords</h2>

<div class="row">
    <div class="col-md-6">
        <div class="feature-box">
            <h4><i class="fa fa-check"></i> Good Password Examples</h4>
            <p><strong>Pattern-based (memorable):</strong></p>
            <ul>
                <li>MyBees2024!Spring</li>
                <li>Honey&Pollen#Hive1</li>
                <li>Queen-Bee$2024</li>
            </ul>
            <p><strong>Passphrase (very strong):</strong></p>
            <ul>
                <li>ILove2CheckMyBeesEveryDay!</li>
                <li>3HivesIn1Apiary#2024</li>
            </ul>
        </div>
    </div>
    <div class="col-md-6">
        <div class="feature-box">
            <h4><i class="fa fa-times"></i> Weak Password Examples</h4>
            <ul>
                <li>password</li>
                <li>beekeeper</li>
                <li>123456</li>
                <li>hive</li>
                <li>myname123</li>
            </ul>
            <p><em>These are easily guessed and provide minimal security.</em></p>
        </div>
    </div>
</div>

<h3>Password Creation Strategies</h3>

<div class="panel panel-default">
    <div class="panel-heading">
        <strong>Method 1: Passphrase</strong>
    </div>
    <div class="panel-body">
        <p>Use a memorable sentence with modifications:</p>
        <ol>
            <li>Think of a sentence: "I check my three hives every morning"</li>
            <li>Take first letters: "Icmt3hem"</li>
            <li>Add numbers and symbols: "Icmt3hem!2024"</li>
            <li>Result: Strong and memorable</li>
        </ol>
    </div>
</div>

<div class="panel panel-default">
    <div class="panel-heading">
        <strong>Method 2: Random with Pattern</strong>
    </div>
    <div class="panel-body">
        <p>Combine random words with numbers and symbols:</p>
        <ol>
            <li>Pick 3 random words: "Honey", "Garden", "Flight"</li>
            <li>Add numbers: "Honey42Garden"</li>
            <li>Add symbols: "Honey42Garden#Flight"</li>
            <li>Result: 22 characters, very strong</li>
        </ol>
    </div>
</div>

<h2>Password Management</h2>

<h3><i class="fa fa-calendar"></i> When to Change Your Password</h3>
<ul>
    <li><strong>Immediately:</strong> If you suspect unauthorized access</li>
    <li><strong>Immediately:</strong> If password was shared accidentally</li>
    <li><strong>Regularly:</strong> Every 90-180 days as a best practice</li>
    <li><strong>After Events:</strong> When team members leave</li>
    <li><strong>Post-Incident:</strong> After any security event</li>
</ul>

<h3><i class="fa fa-key"></i> Password Storage</h3>
<div class="row">
    <div class="col-md-6">
        <h4>Recommended</h4>
        <ul>
            <li>Use a password manager (LastPass, 1Password, Bitwarden)</li>
            <li>Write it down and store in a secure location</li>
            <li>Use browser's password manager (encrypted)</li>
        </ul>
    </div>
    <div class="col-md-6">
        <h4>Not Recommended</h4>
        <ul>
            <li>Plain text file on your computer</li>
            <li>Sticky note on your monitor</li>
            <li>Unencrypted cloud documents</li>
            <li>Email to yourself</li>
        </ul>
    </div>
</div>

<h2>Troubleshooting</h2>

<h3>Forgot Current Password?</h3>
<div class="alert alert-warning">
    <p><strong>If you've forgotten your password:</strong></p>
    <ol>
        <li>Check your password manager or secure notes</li>
        <li>Try variations you commonly use</li>
        <li>If you have database access, you can reset it manually:
            <div class="code-block" style="margin-top: 10px;">
                mysql -u root -p hivecontrol<br>
                UPDATE users SET password=MD5('newpassword') WHERE username='admin';
            </div>
        </li>
        <li>Contact your system administrator if available</li>
        <li>As a last resort, reinstall HiveControl (data can be preserved)</li>
    </ol>
</div>

<h3>Password Change Fails?</h3>
<ul>
    <li><strong>Current password incorrect:</strong> Verify caps lock is off, try again carefully</li>
    <li><strong>New passwords don't match:</strong> Ensure confirmation field matches exactly</li>
    <li><strong>Password too weak:</strong> Add more complexity (numbers, symbols, length)</li>
    <li><strong>Session expired:</strong> Log out and log back in, then try again</li>
    <li><strong>Database error:</strong> Check system logs, verify database is running</li>
</ul>

<h3>Can't Log in After Password Change?</h3>
<ul>
    <li>Verify caps lock is off</li>
    <li>Check you're using the NEW password, not the old one</li>
    <li>Clear browser cache and cookies</li>
    <li>Try a different browser</li>
    <li>Wait 5 minutes - some caching may need to clear</li>
</ul>

<h2>Security Best Practices</h2>

<div class="feature-box">
    <h4><i class="fa fa-shield"></i> Additional Security Measures</h4>
    <ul>
        <li><strong>Unique Passwords:</strong> Use different passwords for different systems</li>
        <li><strong>Two-Factor Authentication:</strong> Consider adding if supported in future</li>
        <li><strong>HTTPS Only:</strong> Always access HiveControl via HTTPS when possible</li>
        <li><strong>Secure Network:</strong> Don't access from public/unsecured WiFi</li>
        <li><strong>Log Out:</strong> Always log out on shared computers</li>
        <li><strong>Monitor Access:</strong> Review login logs periodically</li>
        <li><strong>VPN:</strong> Use VPN when accessing remotely</li>
    </ul>
</div>

<h2>Multi-User Considerations</h2>

<p>If multiple people access your HiveControl system:</p>
<ul>
    <li>Each person should have their own account (if supported)</li>
    <li>Change the shared password if someone loses access</li>
    <li>Document who has access and when they received it</li>
    <li>Rotate passwords when team composition changes</li>
    <li>Consider access levels (view-only vs. admin)</li>
</ul>

<h2>Recovery Planning</h2>

<div class="panel panel-default">
    <div class="panel-heading">
        <strong>Prepare for Password Recovery</strong>
    </div>
    <div class="panel-body">
        <p>Before you forget your password, prepare recovery options:</p>
        <ol>
            <li>Store password in a password manager</li>
            <li>Write down and secure in a safe place</li>
            <li>Share with a trusted backup person (sealed envelope)</li>
            <li>Document database access method for reset</li>
            <li>Keep SSH/terminal access credentials separate and accessible</li>
        </ol>
    </div>
</div>

<h2>Password Policy Recommendations</h2>

<table class="table table-bordered">
    <thead>
        <tr>
            <th>Use Case</th>
            <th>Password Strength</th>
            <th>Change Frequency</th>
        </tr>
    </thead>
    <tbody>
        <tr>
            <td>Personal, local network only</td>
            <td>Medium (10+ chars)</td>
            <td>Yearly</td>
        </tr>
        <tr>
            <td>Personal, internet accessible</td>
            <td>Strong (12+ chars, complex)</td>
            <td>Every 6 months</td>
        </tr>
        <tr>
            <td>Shared/commercial, local network</td>
            <td>Strong (12+ chars, complex)</td>
            <td>Every 90 days</td>
        </tr>
        <tr>
            <td>Shared/commercial, internet</td>
            <td>Very Strong (15+ chars)</td>
            <td>Every 60 days</td>
        </tr>
    </tbody>
</table>

<h2>Quick Reference</h2>

<div class="row">
    <div class="col-md-6">
        <div class="panel panel-success">
            <div class="panel-heading">
                <strong>Password Checklist</strong>
            </div>
            <div class="panel-body">
                <ul style="margin-bottom: 0;">
                    <li>☐ At least 12 characters</li>
                    <li>☐ Mix of upper and lowercase</li>
                    <li>☐ Contains numbers</li>
                    <li>☐ Contains symbols</li>
                    <li>☐ Not a dictionary word</li>
                    <li>☐ Not based on personal info</li>
                    <li>☐ Different from old passwords</li>
                    <li>☐ Stored securely</li>
                </ul>
            </div>
        </div>
    </div>
    <div class="col-md-6">
        <div class="panel panel-info">
            <div class="panel-heading">
                <strong>After Changing Password</strong>
            </div>
            <div class="panel-body">
                <ul style="margin-bottom: 0;">
                    <li>☐ Recorded in password manager</li>
                    <li>☐ Verified can log in with new password</li>
                    <li>☐ Updated any scripts/automation</li>
                    <li>☐ Notified authorized users (if shared)</li>
                    <li>☐ Old password securely discarded</li>
                    <li>☐ Set calendar reminder for next change</li>
                </ul>
            </div>
        </div>
    </div>
</div>
