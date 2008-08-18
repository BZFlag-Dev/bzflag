/* bzflag
* Copyright (c) 1993 - 2008 Tim Riker
*
* This package is free software;  you can redistribute it and/or
* modify it under the terms of the license found in the file
* named COPYING that should have accompanied this file.
*
* THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
* IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
*/

#include "common.h"

bool test_ret(int ret)
{
    if(ret != LDAP_SUCCESS)
    {
        fprintf(stderr, "ERROR: %s\n", ldap_err2string(ret));
        return false;
    }
    else
        return true;
}

#define TEST(exp) if(!test_ret(exp)) return;

struct AttrValPair
{
    char *attr;
    char *vals[2];
};

AttrValPair t[] = {
    {"cn", {"Barbara Jensen", NULL} },
    {"objectClass", {"person", NULL} },
    {"sn", {"Jensen", NULL} },
    {"userPassword", {"asdflaskjasldjkfsdf", NULL} },
    {"description", {"the world's most famous mythical manager", NULL} },
};

#define NUM_ATTRS (sizeof(t)/sizeof(AttrValPair))

LDAP *ld = NULL;

void test_bind()
{
    int version = LDAP_VERSION3;

    TEST( ldap_initialize(&ld, "ldap://127.0.0.1") );
	  TEST( ldap_set_option(ld, LDAP_OPT_PROTOCOL_VERSION, &version) );
    TEST( ldap_simple_bind_s(ld, "cn=Manager,dc=my-domain,dc=com", "secret") );
    printf("LOG: bind successful\n");
}

void test_add()
{
    LDAPMod *attrs[NUM_ATTRS+1];
    LDAPMod attr[NUM_ATTRS];

    for(int i = 0; i < NUM_ATTRS; i++)
    {
        attr[i].mod_op = LDAP_MOD_ADD; 
        attr[i].mod_type = t[i].attr;
        attr[i].mod_values = t[i].vals;
        attrs[i] = &attr[i];
    }
    attrs[NUM_ATTRS] = NULL;

    int msgid;
     TEST( ldap_add_ext(ld, "cn=Barbara Jensen,dc=my-domain,dc=com", attrs, NULL, NULL, &msgid) );

    LDAPMessage *res, *msg;
    ldap_result(ld, msgid, 1, NULL, &res);
    for (msg = ldap_first_message(ld, res); msg; msg = ldap_next_message(ld, msg))
    {
        switch(ldap_msgtype(msg))
        {
            case LDAP_RES_ADD:
                int errcode;
                char *errmsg;
                if(!test_ret( ldap_parse_result(ld, msg, &errcode, NULL, &errmsg, NULL, NULL, 0) ))
                    break;
                if(test_ret(errcode))
                    printf("LOG: add successful\n");
                if(errmsg)
                {
                    if(errmsg[0]) printf("ERROR: %s\n", errmsg);
                    ldap_memfree(errmsg);
                } 
              break;
            default:
              printf("unexpected message type %d\n", ldap_msgtype(msg));
        }
    }

    //printf("LOG: add successful\n");
}

void test_delete()
{
    TEST( ldap_delete_s(ld, "cn=Barbara Jensen,dc=my-domain,dc=com") );
    printf("LOG: delete successful\n");
}

void test_search()
{
    LDAPMessage *res, *msg;
    TEST( ldap_search_s(ld, "dc=my-domain,dc=com", LDAP_SCOPE_SUBTREE, "(objectClass=*)", NULL, 0, &res) );

    for (msg = ldap_first_message(ld, res); msg; msg = ldap_next_message(ld, msg))
    {
        switch(ldap_msgtype(msg))
        {
            case LDAP_RES_SEARCH_ENTRY:
            {
                // print the dn
                printf("LOG: found match\n"); 
                char *dn = ldap_get_dn(ld, msg);
                printf("dn: %s\n", dn);
                ldap_memfree(dn);
                // print the values of the attributes
                BerElement *ber_itr;
                for(char *attr = ldap_first_attribute(ld, msg, &ber_itr); attr; attr = ldap_next_attribute(ld, msg, ber_itr))
                {
                    printf("%s:", attr);
                    char **values = ldap_get_values(ld, msg, attr);
                    int nrvalues = ldap_count_values(values);
                    for(int i = 0; i < nrvalues; i++)
                    {
                        if(i > 0) printf(";");
                        printf(" %s", values[i]);
                    }
                    printf("\n");
                    ldap_value_free(values);
                    ldap_memfree(attr);
                }
                ber_free(ber_itr, 0); // 0 or 1 ?
            } break;
            case LDAP_RES_SEARCH_RESULT:
            {
                int errcode;
                char *matcheddn;
                char *errmsg;
                char **referrals;
                LDAPControl **serverctrls;
                if(!test_ret( ldap_parse_result(ld, msg, &errcode, &matcheddn, &errmsg, &referrals, &serverctrls, 0) ))
                    break;
                if(test_ret(errcode))
                    printf("LOG: search successful\n");

                if(errmsg)
                {
                    if(errmsg[0]) printf("ERROR: %s\n", errmsg);
                    ldap_memfree(errmsg);
                }
                if(matcheddn)
                {
                    if(matcheddn[0]) printf("Matched dn: %s\n", matcheddn);
                    ldap_memfree(matcheddn);
                }
                if(referrals)
                {
                    ldap_value_free(referrals);
                }
                if(serverctrls)
                {
                    ldap_controls_free(serverctrls);
                }
            } break;
            default:
                printf("LOG: unknown message type %d\n", ldap_msgtype(msg));
        }
    }
    
    ldap_msgfree(res);

}

void test_search2()
{
    char *attrs[2] = { LDAP_NO_ATTRS, NULL };
    LDAPMessage *res, *msg;
    TEST( ldap_search_s(ld, "cn=Bawrrbara Jensen,dc=my-domain,dc=com", LDAP_SCOPE_BASE, "(objectClass=*)", attrs, 0, &res) );

    for (msg = ldap_first_message(ld, res); msg; msg = ldap_next_message(ld, msg))
    {
        switch(ldap_msgtype(msg))
        {
            case LDAP_RES_SEARCH_ENTRY:
            {
                // print the dn
                printf("LOG: found match\n"); 
                char *dn = ldap_get_dn(ld, msg);
                printf("dn: %s\n", dn);
                ldap_memfree(dn);
                // print the values of the attributes
                BerElement *ber_itr;
                for(char *attr = ldap_first_attribute(ld, msg, &ber_itr); attr; attr = ldap_next_attribute(ld, msg, ber_itr))
                {
                    printf("%s:", attr);
                    char **values = ldap_get_values(ld, msg, attr);
                    int nrvalues = ldap_count_values(values);
                    for(int i = 0; i < nrvalues; i++)
                    {
                        if(i > 0) printf(";");
                        printf(" %s", values[i]);
                    }
                    printf("\n");
                    ldap_value_free(values);
                    ldap_memfree(attr);
                }
                ber_free(ber_itr, 0); // 0 or 1 ?
            } break;
            case LDAP_RES_SEARCH_RESULT:
            {
                int errcode;
                char *matcheddn;
                char *errmsg;
                char **referrals;
                LDAPControl **serverctrls;
                if(!test_ret( ldap_parse_result(ld, msg, &errcode, &matcheddn, &errmsg, &referrals, &serverctrls, 0) ))
                    break;
                if(test_ret(errcode))
                    printf("LOG: search successful\n");

                if(errmsg)
                {
                    if(errmsg[0]) printf("ERROR: %s\n", errmsg);
                    ldap_memfree(errmsg);
                }
                if(matcheddn)
                {
                    if(matcheddn[0]) printf("Matched dn: %s\n", matcheddn);
                    ldap_memfree(matcheddn);
                }
                if(referrals)
                {
                    ldap_value_free(referrals);
                }
                if(serverctrls)
                {
                    ldap_controls_free(serverctrls);
                }
            } break;
            default:
                printf("LOG: unknown message type %d\n", ldap_msgtype(msg));
        }
    }
    
    ldap_msgfree(res);
}


void test_modify()
{
    char* attr = "description";
    char* values[2] = {"just another gal", NULL};

    LDAPMod mod;
    mod.mod_op = LDAP_MOD_REPLACE;
    mod.mod_type = attr;
    mod.mod_values = values;

    LDAPMod *mods[2] = {&mod, NULL};
    TEST( ldap_modify_s(ld, "cn=Barbara Jensen,dc=my-domain,dc=com", mods) );
    printf("LOG: modify successful\n");
}

void test_unbind()
{
    TEST( ldap_unbind(ld) );
    printf("LOG: unbind successful\n");
}


void test_ldap()
{
    test_bind();
    test_add();
    test_modify();
    test_search();
    test_search2();
    test_delete();
    test_unbind();
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8