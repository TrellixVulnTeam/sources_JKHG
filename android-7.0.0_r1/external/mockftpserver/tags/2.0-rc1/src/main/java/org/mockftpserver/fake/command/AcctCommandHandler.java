/*
 * Copyright 2008 the original author or authors.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
package org.mockftpserver.fake.command;

import org.mockftpserver.core.command.Command;
import org.mockftpserver.core.command.ReplyCodes;
import org.mockftpserver.core.session.Session;
import org.mockftpserver.core.session.SessionKeys;

/**
 * CommandHandler for the ACCT command. Handler logic:
 * <ol>
 * <li>If the required account parameter is missing, then reply with 501</li>
 * <li>If this command was not preceded by a valid USER command, then reply with 503</li>
 * <li>Store the account name in the session and reply with 230</li>
 * </ol>
 *
 * @author Chris Mair
 * @version $Revision: 83 $ - $Date: 2008-07-16 20:32:04 -0400 (Wed, 16 Jul 2008) $
 */
public class AcctCommandHandler extends AbstractFakeCommandHandler {

    protected void handle(Command command, Session session) {
        String accountName = command.getRequiredParameter(0);
        String username = (String) getRequiredSessionAttribute(session, SessionKeys.USERNAME);

        session.setAttribute(SessionKeys.ACCOUNT_NAME, accountName);
        sendReply(session, ReplyCodes.ACCT_OK, "acct", list(username));
    }

}