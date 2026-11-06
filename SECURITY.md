# Security Policy

## Supported Versions

In general, we only support the current stable branch, where the second number of the version is even. For example,
2.**0** and 2.**4** are both stable branches, while 2.**5** is a development branch. After a new major stable release
(such as releasing 2.6.0 when 2.4 had been the stable branch), we will continue to also support the prior stable branch
until the vast majority of players and servers have moved on to the new major release.

| Version | Supported          |
| ------- | ------------------ |
| 2.5.x   | :x:                |
| 2.4.x   | :white_check_mark: |
| < 2.4   | :x:                |

## Reporting a Vulnerability

The BZFlag team takes security reports seriously and appreciates the work that goes into a detailed analysis. If you
discovered a security vulnerability, you can privately report it to the project using the GitHub
["Report a vulnerability"](https://github.com/BZFlag-Dev/bzflag/security/advisories/new) page. This will create a draft
security advisory that will be private until published. We aim to acknowledge all reports within 48 hours.

In the event that GitHub Security Advisories cannot be used, you may also send a report to security@bzexcess.com, but
note that this address is monitored by an individual instead of a team.

Once confirmed, we will work on a fix and discuss publishing the advisory and a new release, and coordinating with other
groups (such as security teams or package maintainers at various Linux distributions) as needed.

We do use a number of third-party dependencies. If you find a vulnerability _in_ one of our dependencies, please report
directly to the maintainers of that project. If, however, the vulnerability is our code improperly using the third-party
dependency, then report the issue to us.
