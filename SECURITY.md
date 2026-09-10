# Security Policy

MetaCall takes security seriously. This policy outlines our supported versions, vulnerability reporting process, and hardening considerations.

---

## Supported Versions

We provide security patches for the following versions:

| Version | Supported | Notes |
| :--- | :---: | :--- |
| `0.5.x` (Latest) | :white_check_mark: | Active development & mainline security fixes |
| `0.4.x` | :white_check_mark: | Critical security patches only |
| `< 0.4.0` | :x: | End of Life (EOL), please upgrade |

---

## Reporting a Vulnerability

**Please do not report security vulnerabilities through public GitHub issues, discussions, or Discord channels.**

### Preferred Reporting Path: GitHub Private Vulnerability Reporting
1. Navigate to the repository's [Security Advisories](https://github.com/metacall/core/security/advisories) tab.
2. Click on **"Report a vulnerability"** to open a private advisory draft.
3. Fill in the details:
   - Affected components (loaders, core runtime, CLI, etc.)
   - Clear reproduction steps or Proof of Concept (PoC)
   - Potential impact of the vulnerability
4. Submit the report.

### Alternative Reporting Path
If you are unable to use GitHub Private Vulnerability Reporting, email our security team directly at **security@metacall.io**.

### Response SLA & Disclosure Process
* **Acknowledgment:** We will acknowledge receipt of your vulnerability report within **48 hours**.
* **Assessment & Fix:** We will keep you informed of our progress while verifying and mitigating the issue.
* **Coordinated Disclosure:** We adhere to responsible and coordinated disclosure. A public advisory and CVE (if applicable) will be released once a patch is ready.
* **Credit:** We publicly credit security researchers who report vulnerabilities responsibly (unless you prefer anonymity).
* **Safe Harbor:** Security research conducted in good faith, adhering to this policy, and without malicious intent or privacy violations will be considered authorized and will not result in legal action.

---

## Hardening & Environment Security

When embedding or running MetaCall in production environments:

* **Path Trust (`LOADER_LIBRARY_PATH` & `LOADER_SCRIPT_PATH`):** Ensure these directories are strictly read-only for unprivileged users to prevent arbitrary code loading.
* **Configuration Permissions (`CONFIGURATION_PATH`):** Lock down configuration directories to prevent unauthorized modification of runtime behavior.
* **Script Verification:** Validate script sources and permissions before dynamic execution.
* **Process & Fork Safety:** Be cautious when using dynamic fork operations within multi-threaded host runtimes.
* **Secret Scanning:** Never commit credentials, API keys, or embedded secrets to scripts loaded via MetaCall.

---

## Security Tooling & Auditing

* **Static Analysis:** Automated code scanning via **CodeQL**.
* **Dependency Monitoring:** Automated supply chain scanning via **Dependabot**.
* **Security Advisories:** Public notices and advisories are published under [MetaCall Core Advisories](https://github.com/metacall/core/security/advisories).